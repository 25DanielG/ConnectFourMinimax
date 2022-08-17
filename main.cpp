#include <iostream>
#include <vector>
#include "./hsrc/board.hpp"
#include "./hsrc/boardgui.hpp"
#include "./hsrc/minmax.hpp"
using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::vector;

int main() {
    string testSequence = "322115133541402206";
    Board tmpBoard;
    tmpBoard.currentGame = testSequence;
    tmpBoard.computeBoard();
    tmpBoard.printBoard();
    vector<vector<char> > matrixBoard = tmpBoard.getMatrixBoard();
    cout << "Final score: " << scoreBoard(tmpBoard, 'X') << endl;
    //output(); Call graphics
}