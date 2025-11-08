#ifndef KDTREE_H
#define KDTREE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include "Words.h"
using namespace std;

//KD-tree over unit-normalized embeddings (cosine == dot)
//build(), knn(q, K) -> vector<pair<index, cosine>>

namespace kd_detail {

/* Cosine and Euclidean for the unit vectors
   - https://en.wikipedia.org/wiki/Cosine_similarity  “Cosine distance” and “L2-normalized Euclidean distance”:
        ||A−B||^2 = 2(1 − cos(A,B)) -> cos_to_dist2(c) = 2 − 2c )
   - https://scikit-learn.org/stable/modules/generated/sklearn.metrics.pairwise.cosine_distances.html
        cosine distance = 1 − cosine similarity; common in NN search with normalized vectors
*/

//cosine for unit vectors == dot product
inline float dot_unit(const vector<float>& a, const vector<float>& b) {
    float s = 0.0f; const size_t n = a.size();
    for(size_t i = 0; i < n; ++i) {
        s += a[i] * b[i];
    }
    return s;
}

//convert cosine to squared Euclidean |q - x|^2 = 2 - 2*(q·x)
inline float cos_to_dist2(float cos_sim) { return 2.0f - 2.0f * cos_sim; }

//pick two pivots that are as dissimilar as possible by cosine
inline pair<int,int> farthest_pair_by_cosine(const vector<int>& idx, const vector<WordVector>& D) {
    if (idx.empty()) return {-1,-1};
    const int a = idx.front();

    auto argmin_dot = [&](int base)->int{
        float best = numeric_limits<float>::infinity();
        int arg = -1;
        for (int id : idx) {
            float v = dot_unit(D[base].vec, D[id].vec);
            if (v < best) { best = v; arg = id; }
        }
        return arg;
    };

    int b = argmin_dot(a);
    if (b < 0) return {a, a};
    int c = argmin_dot(b);
    if (c < 0) return {b, b};
    return {b, c};
}

} //namespace kd_detail

class KDTree {
public:
    struct Node {
        int axis = 0; //split dimension
        float split = 0.0f; //split value
        vector<int> bucket; //leaf indices
        unique_ptr<Node> left;
        unique_ptr<Node> right;
        bool is_leaf() const { return !left && !right; }
    };

    KDTree(const vector<WordVector>& data, size_t leaf_sz = 64)
        : D(data), dim(data.empty() ? 0 : data[0].vec.size()),
          leaf_size(max<size_t>(1, leaf_sz)) {}

    void build() {
        vector<int> idx(D.size());
        for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
        root = build_rec(idx);
    }

    const Node* getRoot() const { return root.get(); }

    //k-NN by cosine returns (index, cosine)
    vector<pair<int,float>> knn(const vector<float>& q, size_t K) const {
        if (K == 0) return {};
        K = min(K, D.size());
        vector<pair<int,float>> best;
        best.reserve(K);
        float min_kept_cos = -1.0f; //worst kept cosine
        knn_rec(root.get(), q, K, best, min_kept_cos);
        return best;
    }

private:
    const vector<WordVector>& D;
    const size_t dim;
    const size_t leaf_size;
    unique_ptr<Node> root;

    /* Pseudocode source: https://en.wikipedia.org/wiki/K-d_tree for construction
        Build rule: choose a split axis then split at the median along that axis then recurse.
        Variants include cycling axes, max-spread axis, midpoint/sliding-midpoint rules.
    Implementation:
        1 If |idx| <= leaf_size -> make a leaf.
        2 Heuristic axis: find two cosine-dissimilar pivots, pick the dimension with largest |p_b[a] − p_c[a]|.
        3 Use nth_element to select the median in O(n) average not full sort.
        4 Partition by axis<split. If one side empty then fall back to alternating split then recurse.
    */


    //building subtree for index set idx
    unique_ptr<Node> build_rec(const vector<int>& idx) {
        if (idx.empty()) return nullptr;

        auto n = make_unique<Node>();

        if (idx.size() <= leaf_size) { //leaf
            n->bucket = idx;
            return n;
        }

        //choose axis using two cosine-dissimilar pivots
        auto [b, c] = kd_detail::farthest_pair_by_cosine(idx, D);
        if (b < 0 || c < 0) { n->bucket = idx; return n; }

        int best_axis = 0; float best_gap = -1.0f;
        for (size_t a = 0; a < dim; ++a) {
            float gap = fabs(D[b].vec[a] - D[c].vec[a]);
            if (gap > best_gap) { best_gap = gap; best_axis = (int)a; }
        }
        n->axis = best_axis;

        //median split along axis
        vector<int> work = idx;
        auto mid_it = work.begin() + work.size()/2;
        nth_element(work.begin(), mid_it, work.end(),
            [&](int i, int j){ return D[i].vec[best_axis] < D[j].vec[best_axis]; });
        n->split = D[*mid_it].vec[best_axis];

        vector<int> L; L.reserve(work.size()/2 + 1);
        vector<int> R; R.reserve(work.size()/2 + 1);
        for (int id : work) {
            (D[id].vec[best_axis] < n->split ? L : R).push_back(id);
        }
        if (L.empty() || R.empty()) { //ensuring both sides are not empty
            L.clear(); R.clear();
            for (size_t t = 0; t < work.size(); ++t) (t & 1 ? L : R).push_back(work[t]);
        }

        n->left  = build_rec(L);
        n->right = build_rec(R);
        return n;
    }

    /* Pseudocode source: https://en.wikipedia.org/wiki/K-d_tree for nearest neighbour search
    KD-tree NN search:
        Descend to the leaf that would contain q by split axis and split value.
        Track current best is top k by cosine.
        Visit the "far" side only if the splitting plane could contain a better point.
    Cosine adaptation:
        Similarity = dot(q, x).
        Maintain min_kept_cos among K best.
        Plane-crossing test is if (q[a]−split)^2 <= best_dist2, where best_dist2 = 2 − 2*min_kept_cos, then the far branch might improve the result then recurse there.
    */

    //search
    void knn_rec(const Node* node, const vector<float>& q, size_t K, vector<pair<int,float>>& best, float& min_kept_cos) const {
        if (!node) return;

        if (node->is_leaf()) {
            for (int id : node->bucket) {
                float cs = kd_detail::dot_unit(q, D[id].vec);
                if (best.size() < K) {
                    best.emplace_back(id, cs);
                    if (best.size() == K) {
                        sort(best.begin(), best.end(),
                                  [](auto& a, auto& b){ return a.second > b.second; });
                        min_kept_cos = best.back().second;
                    }
                } else if (cs > min_kept_cos) {
                    auto it = upper_bound(
                        best.begin(), best.end(), cs,
                        [](float v, const pair<int,float>& p){ return v > p.second; }
                    );
                    best.insert(it, {id, cs});
                    best.pop_back();
                    min_kept_cos = best.back().second;
                }
            }
            return;
        }

        const int a = node->axis;
        const Node* near = (q[a] < node->split ? node->left.get()  : node->right.get());
        const Node* far  = (q[a] < node->split ? node->right.get() : node->left.get());

        //near first
        knn_rec(near, q, K, best, min_kept_cos);

        //visit far if distance allows
        float diff = q[a] - node->split;
        float best_dist2 = (best.size() < K)
            ? numeric_limits<float>::infinity()
            : kd_detail::cos_to_dist2(min_kept_cos);
        if (diff*diff <= best_dist2) {
            knn_rec(far, q, K, best, min_kept_cos);
        }
    }
};

#endif // KDTREE_H