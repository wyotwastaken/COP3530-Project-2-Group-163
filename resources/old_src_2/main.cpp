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

    // Construct Ball Tree
    cout << "Constructing ball tree..." << endl;
    BallTree ball_tree;
    ball_tree.constructBalltree(words.getWords(), words);
    cout << "Ball tree constructed!" << endl;

    cout << "Root radius: " << ball_tree.getRoot()->getRadius() << endl;
    return 0;
}