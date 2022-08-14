#ifndef BOARD_H_
#define BOARD_H_
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
        void getMove(); // Gets the move from the player
};
#endif