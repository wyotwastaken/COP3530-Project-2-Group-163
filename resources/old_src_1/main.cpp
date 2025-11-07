#include <iostream>
#include "BallTree.h"
#include "Words.h"
using namespace std;

int main() {
    Words words;
    words.loadWords("/Users/wyattscheinbaum/CLionProjects/COP3530-Project-2-Group-163/data/word_list.txt");
    cout << "Loaded " << words.getWords().size() << " words!" << endl;
    words.printWordsRange(10);
    cout << endl;

    // Example cosine similarity usage. Range: [-1, 1]. ~1 implies semantically similar, ~0 implies semantically unrelated, ~-1 implies semantically dissimilar
    cout << "cosine similarity of 'the' and 'a': " << words.cosine_similarity("the", "a");

    // Construct Ball Tree
    BallTree ball_tree;
    ball_tree.constructBalltree(words.getWords());

    cout << "Root radius: " << ball_tree.getRoot()->getRadius() << endl;
    return 0;
}