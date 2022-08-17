#include <iostream>
#include "../hsrc/board.hpp"
#include <vector>
#include <map>
using std::cout;
using std::endl;
using std::vector;
using std::string;
void Board::computeBoard() { // Converts the board from a board sequence to a 2d array
    vector<vector<char> > computedBoard(rows, vector<char>(columns, '#'));
    bool playerOne = true;
    std::map<int, int> rowTracker;
    for(int i = 0; i < columns; ++i) {
        rowTracker.insert(std::pair<int, int>(i, rows - 1));
    }
    for(int i = 0; i < currentGame.length(); ++i) {
        int currentMove = currentGame[i] - '0';
        if(playerOne)
            computedBoard[rowTracker[currentMove]][currentMove] = 'X';
        else
            computedBoard[rowTracker[currentMove]][currentMove] = 'O';
        --rowTracker[currentMove];
        playerOne = !playerOne;
    }
    currentComputedBoard = computedBoard;
}
void Board::printBoard() { // Prints the board given the 2d board array
    cout << "________________________" << endl;
    for(int r = 0; r < rows; ++r) {
        cout << "| ";
        for(int c = 0; c < columns; ++c) {
            cout << currentComputedBoard[r][c] << "  ";
        }
        cout << "|" << endl;
    }
}
void Board::getMove() {
    cout << "Enter the column where you want to play a piece: ";
    int move;
    std::cin >> move;
    cout << endl;
    currentGame += move;
}
vector<vector<char> > Board::getMatrixBoard() { // Gets the board from a game sequence to a matrix
    vector<vector<char> > computedBoard(rows, vector<char>(columns, '#'));
    bool playerOne = true;
    std::map<int, int> rowTracker;
    for(int i = 0; i < columns; ++i) {
        rowTracker.insert(std::pair<int, int>(i, rows - 1));
    }
    for(int i = 0; i < currentGame.length(); ++i) {
        int currentMove = currentGame[i] - '0';
        if(playerOne)
            computedBoard[rowTracker[currentMove]][currentMove] = 'X';
        else
            computedBoard[rowTracker[currentMove]][currentMove] = 'O';
        --rowTracker[currentMove];
        playerOne = !playerOne;
    }
    return computedBoard;
}