#ifndef WORDS_H
#define WORDS_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream> // For istringstream, referenced from https://cplusplus.com/reference/sstream/istringstream/str
#include <cmath>
using namespace std;

// Single word object.
struct WordVector {
  string word;
  void setWord(string word) {this->word = word;}
  string getWord() const {return this->word;}

  // Float vector of dimension 100.
  vector<float> vec;
  void setVec(vector<float> vec) {this->vec = vec;}
  vector<float> getVec() const {return this->vec;}
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

    // Normalize each WordVector - v_norm = v / ||v|| = v / sum. If sum = 0, continue.
    for (int k = 0; k < words[i].vec.size(); k++) {
      if (sum == 0) {
        continue;
      }
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
    return;
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
  cout << "Error: Word not found" << endl;
  return WordVector();
}

#endif //WORDS_H
