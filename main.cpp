#include <iostream>
#include <vector>
#include <string>
#include "./hsrc/board.hpp"
#include "./hsrc/boardgui.hpp"
#include "./hsrc/minmax.hpp"
#include "./hsrc/treeNode.hpp"
using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::vector;

int main() {
    Board tmpBoard;
    int depth = 4;
    tmpBoard.currentGame = "";
    bool computer = false;
    while(true) {
        tmpBoard.computeBoard();
        cout << "Board:" << endl;
        tmpBoard.printBoard();
        if(computer) {
            treeNode<Board> tree; 
            tree.setValue(tmpBoard);
            createTree(tree, depth);
            cerr << "Created tree and entered minimax" << endl;
            int nextMove = (minimax(tree, depth, true, INT32_MIN)).second;
            cerr << "Return of minimax: " << nextMove << endl;
            tmpBoard.currentGame += std::to_string(nextMove);
            cerr << "Current game is: " << tmpBoard.currentGame << endl;
        } else {
            cout << "Enter a column to move in: ";
            int columnMove;
            std::cin >> columnMove;
            tmpBoard.currentGame += std::to_string(columnMove);
            cerr << "Current game is: " << tmpBoard.currentGame << endl;
        }
        computer = !computer;
    }
    //output(); Call graphics
}