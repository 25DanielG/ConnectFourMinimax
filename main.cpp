#include <iostream>
#include <vector>
#include <string>
#include "./hsrc/board.hpp"
#include "./hsrc/boardgui.hpp"
#include "./hsrc/minmax.hpp"
using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::vector;

int main() {
    const int depth = 4;
    Board gameBoard; // Declare board
    bool computer = false;
    while(true) {
        gameBoard.computeBoard();
        cout << "Board:" << endl;
        gameBoard.printBoard();
        if(computer) {
            cerr << endl << "Entered minimax" << endl;
            int nextMove = minimax(gameBoard, depth, true).second;
            cerr << "Return of minimax: " << nextMove << endl;
            gameBoard.currentGame += std::to_string(nextMove);
            cerr << "Current game is: " << gameBoard.currentGame << endl;
        } else {
            cout << "Enter a column to move in: ";
            int columnMove;
            std::cin >> columnMove;
            gameBoard.currentGame += std::to_string(columnMove);
            cerr << "Current game is: " << gameBoard.currentGame << endl;
        }
        computer = !computer;
    }
    // output(); // Call graphics
}