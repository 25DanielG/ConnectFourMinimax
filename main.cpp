#include <iostream>
#include "./hsrc/board.hpp"
#include "./hsrc/boardgui.hpp"
using std::endl;
using std::cout;
using std::cerr;
using std::string;

int main() {
    string testSequence = "4453";
    Board tmpBoard;
    cerr << "Created the board object" << endl;
    tmpBoard.currentGame = testSequence;
    cerr << "Inserted test sequence" << endl;
    tmpBoard.computeBoard();
    cerr << "Computed Board" << endl;
    tmpBoard.printBoard();
    output();
}