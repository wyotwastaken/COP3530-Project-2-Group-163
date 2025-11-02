#ifndef WORDS_H
#define WORDS_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream> // For istringstream, referenced from https://cplusplus.com/reference/sstream/istringstream/str
using namespace std;

// Single word object. Contains a word and a float vector of dim = 100.
struct WordVector {
  string word;
  vector<float> vec;
  void setWord(string word) {this->word = word;}
  void setVec(vector<float> vec) {this->vec = vec;}
  string getWord() {return this->word;}
  vector<float> getVec() {return this->vec;}
};

// Loads words and vectors from the GloVe txt file.
class Words {
  private:
    vector<WordVector> words;
  public:
    void loadWords(string fileName);
    void normalizeWords(); // For cosine similarity computation
    void printWords();
    void printWordsRange(int range);
    WordVector findWord(string w);
    float cosine_similarity(string word1, string word2);
    const vector<WordVector>& getWords() {return words;}
};

void Words::normalizeWords() {
  for (int i = 0; i < words.size(); i++) {
    // Compute magnitude - ||v|| = sqrt(v_1^2 + v_2^2 + ... + v_n^2)
    float sum = 0;
    for (int j = 0; j < words[i].vec.size(); j++) {
      float num = words[i].vec[j];
      sum += (num * num);
    }
    sum = sqrt(sum);

    // Normalize each WordVector - v_norm = v / ||v|| = v / sum
    for (int k = 0; k < words[i].vec.size(); k++) {
      words[i].vec[k] = words[i].vec[k] / sum;
    }
  }
}

// Referenced https://cplusplus.com/reference/sstream/istringstream/str for istringstream (iss) usage.
void Words::loadWords(string fileName) {
  ifstream wordstxt;
  wordstxt.open(fileName);
  if (!wordstxt.is_open()) {
    cerr << "Error opening file " << fileName << endl;
  }

  string line;
  while (getline(wordstxt, line)) {
    WordVector w = WordVector();
    istringstream iss(line);

    // Set word
    string word;
    iss >> word;
    w.setWord(word);

    // Set vector
    vector<float> vec;
    float val;
    for (int i = 0; i < 100; i++){
      iss >> val;
      vec.push_back(val);
    }
    w.setVec(vec);
    words.push_back(w);
  }

  // End by normalizing the word vectors for easy cosine similarity computation cos_sim(u, v) = u_norm (dot) v_norm.
  normalizeWords();
}

void Words::printWords() {
  for (int i = 0; i < words.size(); i++) {
    cout << "Word: " << words[i].word << endl;
    cout << "Vector: ";
    for (int j = 0; j < words[i].vec.size(); j++) {
      cout << words[i].vec[j] << ", ";
    }
  }
}

void Words::printWordsRange(int range) {
  for (int i = 0; i < range; i++) {
    cout << "Word: " << words[i].word << endl;
    cout << "Vector: ";
    for (int j = 0; j < words[i].vec.size(); j++) {
      cout << words[i].vec[j] << ", ";
    }
    cout << endl;
    cout << endl;
  }
}

// Returns a WordVector of a particular word. If its not found, the first word is returned.
WordVector Words::findWord(string w) {
  for (int i = 0; i < words.size(); i++) {
    if (words[i].word == w) {
      return words[i];
    }
  }
  return words[0];
}

// cosine_similarity(u, v) = u_norm (dot) v_norm = ((u_norm_1 * v_norm_1) + (u_norm_2 * v_norm_2) * ... * (u_norm_n * v_norm_n))
float Words::cosine_similarity(string word1, string word2) {
  WordVector w1 = findWord(word1);
  WordVector w2 = findWord(word2);
  vector<float> vec1 = w1.vec;
  vector<float> vec2 = w2.vec;
  float sum = 0;
  for (int i = 0; i < vec1.size(); i++) {
    sum += (vec1[i] * vec2[i]);
  }
  return sum;
}

#endif //WORDS_H
