#ifndef BOARD_H
#define BOARD_H
#include <iostream>
#include <vector>
#include <map>
using std::cout;
using std::endl;
using std::vector;
using std::string;
class Board {
    public:
        const int rows = 6;
        const int columns = 7;
        string currentGame = "";
        vector<vector<char> > currentComputedBoard;
        Board() { // Constructor
    
        }
        void computeBoard() { // Converts the board from a board sequence to a 2d array
            vector<vector<char> > computedBoard(rows, vector<char>(columns, '#'));
            bool playerOne = true;
            std::map<int, int> rowTracker;
            for(int i = 0; i < columns; ++i) {
                rowTracker.insert(std::pair<int, int>(i, rows - 1));
            }
            for(int i = 0; i < currentGame.length(); ++i) {
                int currentMove = currentGame[i] - '0';
                cout << "Currentmove: " << currentMove << endl;
                if(playerOne)
                    computedBoard[rowTracker[currentMove]][currentMove] = 'X';
                else
                    computedBoard[rowTracker[currentMove]][currentMove] = 'O';
                --rowTracker[currentMove];
                playerOne = !playerOne;
            }
            currentComputedBoard = computedBoard;
        }
        void printBoard() { // Prints the board given the 2d board array
            cout << "________________________" << endl;
            for(int r = 0; r < rows; ++r) {
                cout << "| ";
                for(int c = 0; c < columns; ++c) {
                    cout << currentComputedBoard[r][c] << "  ";
                }
                cout << "|" << endl;
            }
        }
};
#endif