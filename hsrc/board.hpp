#ifndef BOARD_H
#define BOARD_H
#include <vector>
class Board {
    public:
        const int rows = 6;
        const int columns = 7;
        std::string currentGame = "";
        std::vector<std::vector<char> > currentComputedBoard;
        Board() { // Constructor
    
        }
        void computeBoard(); // Converts the board from a board sequence to a 2d array
        void printBoard(); // Prints the board given the 2d board array
};
#endif