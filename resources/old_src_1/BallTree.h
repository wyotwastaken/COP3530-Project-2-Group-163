#ifndef BALLTREE_H
#define BALLTREE_H
#include <iostream>
#include "Words.h"
#include <vector>
using namespace std;

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
    Words words;
    int max_leaf_size = 20; // Can be changed.
  public:
    // Helper Functions:
    // Computes the lowest cosine similarity (ie closest to -1) comparing an input word to other vectors.
    WordVector lowestCosSimilarity(WordVector input_word, vector<WordVector> word_list);

    // Normalizes an input vector
    vector<float> normalize(vector<float>& input);

    // Returns the average vector of a vector of WordVectors
    vector<float> average(vector<WordVector> input_words);

    // Computes the cosine similarity of the pivot and some WordVector
    float pivot_word_cosine_similarity(vector<float> p, WordVector w);

    // Getters:
    BallTreeNode *getRoot() {return root;}

    // Main Methods:
    BallTreeNode* constructBalltree(vector<WordVector> words);
};

WordVector BallTree::lowestCosSimilarity(WordVector input_word, vector<WordVector> word_list) {
  WordVector most_semantically_dissimilar;
  float lowest_cos_similarity = 1;
  for (int i = 0; i < word_list.size(); i++) {
    float cos_sim = words.cosine_similarity(input_word.getWord(), word_list[i].getWord());
    if (cos_sim < lowest_cos_similarity) {
      most_semantically_dissimilar = word_list[i];
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

vector<float> BallTree::average(vector<WordVector> input_words) {
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

float BallTree::pivot_word_cosine_similarity(vector<float> p, WordVector w) {
  float sum = 0;
  for (int i = 0; i < p.size(); i++) {
    sum += (p[i] * w.vec[i]);
  }
  return sum;
}


// Psuedocode source: https://en.wikipedia.org/wiki/Ball_tree
// funcction constructBalltree is input: D, an array of data points. output: B, the root of the constructed ball tree.
BallTreeNode* BallTree::constructBalltree(vector<WordVector> words) {
  /* Base case:
  if a single point remains then
    create a leaf B containing a single point in D
    return B
  */
  if (words.size() == 0) {
    return nullptr;
  }
  if (words.size() == 1) {
    BallTreeNode *root = new BallTreeNode();
    root->setWords(words);
    return root;
  }

  /*
  else:
  let c be the dimension of greatest spread
        let p be the central point selected considering c
        let L, R be the sets of points lying to the left and right of the median along dimension c
        create B with two children:
            B.pivot := p
            B.child1 := construct_balltree(L),
            B.child2 := construct_balltree(R),
            let B.radius be maximum distance from p among children
        return B
  */
  else {
    // Instantiate new root node
    BallTreeNode *root = new BallTreeNode();

    // Calculate the spread. For cosine similarity, its 2 points with the greatest angular distance.
    // Logic for spread calculation obtained from 18:55 in https://www.youtube.com/watch?v=E1_WCdUAtyE
    WordVector A = lowestCosSimilarity(words[0], words);
    WordVector B = lowestCosSimilarity(A, words);

    // let p be the central point selected considering c
    // In this case, p is the normalized average of all vectors in words.
    // Use cosine distance for a "radius" measure. Source I used to learn about cosine distance: https://medium.com/@milana.shxanukova15/cosine-distance-and-cosine-similarity-a5da0e4d9ded
    vector<float> p = average(words);
    normalize(p);
    root->center = p;
    float max_cos_distance = 0;
    for (int i = 0; i < words.size(); i++) {
      float num = pivot_word_cosine_similarity(p, words[i]);
      if (1-num > max_cos_distance) {
        max_cos_distance = 1-num; // Cosine distance
      }
    }
    root->radius = max_cos_distance;

    // let L, R be the sets of points lying to the left and right of the median along dimension c
    vector<WordVector> L;
    vector<WordVector> R;
    for (int i = 0; i < words.size(); i++) {
      if (this->words.cosine_similarity(A.getWord(), words[i].getWord()) > this->words.cosine_similarity(B.getWord(), words[i].getWord())) {
        L.push_back(words[i]);
      }
      else {
        R.push_back(words[i]);
      }
    }
    // B.pivot := p
    root->center = p;

    // create B with two children:
    root->left = constructBalltree(L);
    root->right = constructBalltree(R);
    return root;
  }
}

#endif //BALLTREE_H
