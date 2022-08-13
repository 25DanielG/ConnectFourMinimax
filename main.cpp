#include <iostream>
#include "./src/board.hpp"

int main() {
    std::string testSequence = "4453";
    Board tmpBoard;
    std::cerr << "Created the board object" << endl;
    tmpBoard.currentGame = testSequence;
    std::cerr << "Inserted test sequence" << endl;
    tmpBoard.computeBoard();
    std::cerr << "Computed Board" << endl;
    tmpBoard.printBoard();
}