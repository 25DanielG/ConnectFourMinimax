#ifndef BOARD_HPP_
#define BOARD_HPP_
#include <vector>
using std::vector;
class Board {
    public:
        static const int rows = 6;
        static const int columns = 7;
        std::string currentGame = "";
        std::vector<std::vector<char> > currentComputedBoard;
        Board() { // Constructor
    
        }
        void computeBoard(); // Converts the board from a board sequence to a 2d array
        void printBoard(); // Prints the board given the 2d board array
        void getMove(); // Gets the move from the player
        vector<vector<char> > getMatrixBoard(); // Gets the board from a game sequence to a matrix
};
#endif