#ifndef BALLTREE_H
#define BALLTREE_H
#include <iostream>
#include "Words.h"
#include <vector>
#include <cmath>
#include <queue>
using namespace std;

// Object for comparing distances and WordVectors.
struct knn_Node {
  float distance;
  WordVector word;

  knn_Node(float d, WordVector& w) : distance(d), word(w) {}

  bool operator< (const knn_Node &other) const {
    return distance < other.distance;
  }
};

struct BallTreeNode {
  BallTreeNode *left; // Left ball
  BallTreeNode *right; // Right ball
  float radius; // Radius of ball
  vector<float> center; // Center point - some vector containing a string and 100 dimension values
  vector<WordVector> words; // If this node is a leaf, it will have a vector of points contained within the sphere.

  // Main Methods
  BallTreeNode() : left(nullptr), right(nullptr), radius(0.0) {} // Constructor
  void setWords(vector<WordVector> words) {this->words = words;}
  float getRadius() {return radius;}
};

class BallTree {
  private:
    BallTreeNode *root;
    int max_leaf_size = 20; // Can be changed. Currently being not used
  public:
    // Helper Functions:
    // Computes the lowest cosine similarity (ie closest to -1) comparing an input word to other vectors.
    WordVector lowestCosSimilarity(const WordVector input_word, const vector<WordVector> word_list);

    // Normalizes an input vector
    vector<float> normalize(vector<float>& input);

    // Returns the average vector of a vector of WordVectors
    vector<float> average(const vector<WordVector>& input_words);

    // Computes the cosine similarity of two vectors
    float cosine_similarity(const vector<float>& a, const vector<float>& b);

    // Computes the cosine distance of two vectors (1 - cosine_similarity(a, b))
    float cosine_distance(const vector<float>& a, const vector<float>& b);

    // Main ball tree constructor:
    BallTreeNode* constructBalltreeHelper(const vector<WordVector>& words, Words& all_words);

    // KNN search algorithm:
    void knn_search_helper(const WordVector t, int k, priority_queue<knn_Node>& Q, BallTreeNode* B);

    // Getters:
    BallTreeNode *getRoot() {return root;}

    // Main Methods:
    void constructBalltree(const vector<WordVector>& words, Words& all_words);
    priority_queue<knn_Node> knn_search(const WordVector t, int k);
};

WordVector BallTree::lowestCosSimilarity(const WordVector input_word, const vector<WordVector> word_list_vector) {
  WordVector most_semantically_dissimilar;
  float lowest_cos_similarity = 1;
  for (int i = 0; i < word_list_vector.size(); i++) {
    float cos_sim = cosine_similarity(input_word.vec, word_list_vector[i].vec);
    if (cos_sim < lowest_cos_similarity) {
      most_semantically_dissimilar = word_list_vector[i];
      lowest_cos_similarity = cos_sim;
    }
  }
  return most_semantically_dissimilar;
}

vector<float> BallTree::normalize(vector<float>& input) {
  float sum = 0;
  for (int i = 0; i < input.size(); i++) {
    float num = input[i] * input[i];
    sum += num;
  }
  sum = sqrt(sum);
  for (int j = 0; j < input.size(); j++) {
    input[j] = input[j] / sum;
  }
  return input;
}

vector<float> BallTree::average(const vector<WordVector>& input_words) {
  vector<float> output(100);
  for (int i = 0; i < input_words.size(); i++) {
    for (int j = 0; j < input_words[i].vec.size(); j++) {
      output[j] += input_words[i].vec[j];
    }
  }
  for (int k = 0; k < output.size(); k++) {
    output[k] = output[k] / input_words.size();
  }
  return output;
}

float BallTree::cosine_similarity(const vector<float>& a, const vector<float>& b) {
  float sum = 0;
  for (int i = 0; i < a.size(); i++) {
    sum += (a[i] * b[i]);
  }
  return sum;
}

/* Psuedocode source: https://en.wikipedia.org/wiki/Ball_tree
    Translated to cosine similarity/distance (project context):
    1. Instantiate new root node
    2. Calculate the spread. For cosine similarity, its 2 points with the greatest angular distance.
       (Logic for spread calculation obtained from 18:55 in https://www.youtube.com/watch?v=E1_WCdUAtyE)
    3. "let p be the central point selected considering c"
        In this case, p is the normalized average of all vectors in words.
        Use cosine distance for a "radius" measure. Source I used to learn about cosine distance: https://medium.com/@milana.shxanukova15/cosine-distance-and-cosine-similarity-a5da0e4d9ded
    4. "let L, R be the sets of points [with a with cosine similarities closest to A or B, respectively] along [spread A,B]"
    5. "B.pivot := p" == root.center := p (pivot)
    6. Check if L or R are empty to prevent infinite recursion. IMPORTANT: Since this is a leaf, the words must be set.
    7. Create B with two children:
       "B.child1 := construct_balltree(L)" (root->left)
       "B.child2 := construct_balltree(R)" (root->right)
*/
BallTreeNode* BallTree::constructBalltreeHelper(const vector<WordVector>& words, Words& all_words) {
  if (words.size() == 0) {
    return nullptr;
  }
  if (words.size() <= max_leaf_size) {
    BallTreeNode *root = new BallTreeNode();
    root->setWords(words);

    // Compute the center vector and radius of leaf nodes for knn_search.
    vector<float> p = average(words);
    normalize(p);
    float max_cos_distance = 0;
    for (int i = 0; i < words.size(); i++) {
      float num = cosine_similarity(p, words[i].vec);
      if (1-num > max_cos_distance) {
        max_cos_distance = 1-num; // Cosine distance
      }
    }
    root->radius = max_cos_distance;
    root->center = p;

    return root;
  }
  else {
    // (1)
    BallTreeNode *root = new BallTreeNode();
    // (2)
    WordVector A = lowestCosSimilarity(words[0], words);
    WordVector B = lowestCosSimilarity(A, words);
    // (3)
    vector<float> p = average(words);
    normalize(p);
    float max_cos_distance = 0;
    for (int i = 0; i < words.size(); i++) {
      float num = cosine_similarity(p, words[i].vec);
      if (1-num > max_cos_distance) {
        max_cos_distance = 1-num; // Cosine distance
      }
    }
    root->radius = max_cos_distance;
    // (4)
    vector<WordVector> L;
    vector<WordVector> R;
    for (int i = 0; i < words.size(); i++) {
      if (cosine_similarity(A.vec, words[i].vec) > cosine_similarity(B.vec, words[i].vec)) {
        L.push_back(words[i]);
      }
      else {
        R.push_back(words[i]);
      }
    }
    // (5)
    root->center = p;
    // (6)
    // cout << "Splitting " << words.size() << " words..." << endl; // remove later
    if (words.size() > 100000) {
      cout << "." << flush;
    }
    if (L.empty() || R.empty()) {
      root->setWords(words);
      return root;
    }
    // (7)
    root->left = constructBalltreeHelper(L, all_words);
    root->right = constructBalltreeHelper(R, all_words);
    return root;
  }
}

void BallTree::constructBalltree(const vector<WordVector>& words, Words& all_words) {
  root = constructBalltreeHelper(words, all_words);
}

float BallTree::cosine_distance(const vector<float>& a, const vector<float>& b) {
  return 1 - cosine_similarity(a, b);
}

/* Psuedocode source: https://en.wikipedia.org/wiki/Ball_tree
    Translated to project context:
    Note: Objects are stored as knn_Nodes, each having a calculated distance, WordVector, and < operator for min heap processing.
    1) return immediately if B is a nullptr to avoid segfault.
    2) else if B.left == nullptr and B.right == nullptr (B is a leaf) then, for each WordVector w in B.words:
    2a) if cosine_distance(t.vec, w.vec) < cosine_distance(t.vec, Q.top().word.vec) then Q.push(w).
    2b) if 2a was satisfied and Q.size() > k, then remove the element of Q with the greatest distance value.
    3) if cosine_distance(t.vec, B.center) - B.radius >= cosine_distance(t.vec, Q.top().word.vec) then return Q unchanged.
    (repeat for each WordVector x in B.words)
    4) else (if neither 1 nor 2 were satisfied):
    4a) if cosine_distance(t.vec, B.left.center) < cosine_distance(t.vec, B.right.center), then child1 = B.left, child2 = B.right.
    4b) else child1 = B.right, child2 = B.left
    5) recursively call knn_search(t, k, Q, child1) followed by knn_search(t, k, Q, child2).
 */
void BallTree::knn_search_helper(const WordVector t, int k, priority_queue<knn_Node>& Q, BallTreeNode* B) {
  // (1)
  if (B == nullptr) {
    return;
  }
  // (2)
  if (B->left == nullptr && B->right == nullptr) {
    for (WordVector &w : B->words) {
      // (2a)
      float cos_dist = cosine_distance(t.vec, w.vec);
      if (cos_dist < cosine_distance(t.vec, Q.top().word.vec)) {
        Q.push(knn_Node(cos_dist, w));
      }
      // (2b)
      if (Q.size() > k) {
        Q.pop();
      }
    }
  }
  // (3)
  else if (cosine_distance(t.vec, B->center) - B->radius >= cosine_distance(t.vec, Q.top().word.vec)) {
    return;
  }
  // (4)
  else {
    BallTreeNode* child1;
    BallTreeNode* child2;
    // (4a)
    if (cosine_distance(t.vec, B->left->center) < cosine_distance(t.vec, B->right->center)) {
      child1 = B->left;
      child2 = B->right;
    }
    // (4b)
    else {
      child1 = B->right;
      child2 = B->left;
    }
    // (5)
    knn_search_helper(t, k, Q, child1);
    knn_search_helper(t, k, Q, child2);
  }
}

priority_queue<knn_Node> BallTree::knn_search(const WordVector t, int k) {
  priority_queue<knn_Node> Q;
  cout << "Searching for " << t.getWord() << "'s nearest semantic neighbors..." << endl;
  if (k <= 0) {
    cout << "Error: knn search must be non-negative." << endl;
    return Q;
  }
  cout << endl;

  for (int i = 0; i < k; i++) {
    WordVector w;
    Q.push(knn_Node(3, w));
  }
  knn_search_helper(t, k+1, Q, getRoot());
  int rank = 1;
  vector<string> top_k_words;
  vector<float> top_k_similarities;
  cout << "Top " << k << " words semantically similar to " << t.getWord() << ":" << endl;
  while (!Q.empty()) {
    top_k_words.push_back(Q.top().word.getWord());
    top_k_similarities.push_back(cosine_similarity(Q.top().word.vec, t.vec));
    Q.pop();
  }
  int vec_size = top_k_words.size();
  for (int i = vec_size-1; i > 0; i--) {
    cout << "[" << rank++ << "] " << top_k_words[i] << " (similarity: " << top_k_similarities[i] << ")" << endl;
  }
  return Q;
}

#endif //BALLTREE_H
