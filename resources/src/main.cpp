#include <iostream>
#include <chrono>
#include "BallTree.h"
#include "Words.h"
using namespace std;

int main() {
    //// BEFORE RUNNING ////
    // 1) Drop word_list.txt into data folder.
    // 2) CHANGE word_txt to correct path under your data folder.
    string word_txt = "/Users/wyattscheinbaum/CLionProjects/COP3530-Project-2-Group-163/data/word_list.txt";

    Words words;
    cout << "Loading words" << flush; // Read about std::flush here: https://en.cppreference.com/w/cpp/io/manip/flush.html

    // chrono usage referenced from https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c.
    auto t1 = chrono::high_resolution_clock::now();
    words.loadWords(word_txt);
    auto t2 = chrono::high_resolution_clock::now();

    auto ms_int = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    auto s_int = chrono::duration_cast<chrono::seconds>(t2 - t1).count();

    cout << endl;
    cout << "Loaded " << words.getWords().size() << " words!" << endl;
    cout << "Execution time: " << ms_int << " milliseconds. (" << s_int << " seconds)" << endl;

    // Construct Ball Tree
    cout << "Constructing ball tree" << flush;
    BallTree ball_tree;

    auto t3 = chrono::high_resolution_clock::now();
    ball_tree.constructBalltree(words.getWords(), words);
    auto t4 = chrono::high_resolution_clock::now();

    auto ms_int_2 = chrono::duration_cast<chrono::milliseconds>(t4 - t3).count();
    auto s_int_2 = chrono::duration_cast<chrono::seconds>(t4 - t3).count();

    cout << endl;
    cout << "Ball tree constructed!" << endl;
    cout << "Execution time: " << ms_int_2 << " milliseconds. (" << s_int_2 << " seconds)" << endl;

    // Semantic knn search input
    bool running = true;
    string w = "";
    string neighbors = "";
    while (running) {
        cout << "Enter new word to generate semantic neighbor list (type '0' to exit): ";
        cin >> w;
        if (w == "0") {
            running = false;
            break;
        }

        // Find w's WordVector
        WordVector t = words.findWord(w);
        if (t.getWord() == "") {
            cout << "Please enter a valid word..." << endl;
            continue;
        }

        cout << "Enter number of neighbors to search: ";
        cin >> neighbors;
        int k = stoi(neighbors);

        auto t5 = chrono::high_resolution_clock::now();
        ball_tree.knn_search(t, k);
        auto t6 = chrono::high_resolution_clock::now();

        auto ms_int_3 = chrono::duration_cast<chrono::milliseconds>(t6 - t5).count();
        auto s_float_3 = chrono::duration_cast<chrono::seconds>(t6 - t5).count();

        cout << endl;
        cout << "Execution time: " << ms_int_3 << " milliseconds" << endl;
        cout << endl;
    }
    return 0;
}