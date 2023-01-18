#ifndef MINMAX_HPP_
#define MINMAX_HPP_
#include <vector>
#include <iostream>
#include "./board.hpp"
#define minimaxDepth 7
#define NUM_COLUMNS 7
#define NUM_ROWS 8
#define NO_MOVE -1
struct coordDirection {
    std::pair<int, int> coordinate;
    std::string direction;
};
struct winInfo {
    coordDirection coords;
    int winCol;
};
struct minimaxValues {
    Board board;
    unsigned int depth;
    bool maximizingPlayer;
    int alpha;
    int beta;
    unsigned int move;
};
void performMove(Board gameBoard); // Performs a move
void trainComputer(int train_depth); // Plays the comptuer against itself in order to expand the transposition table
std::pair<int, int> threading(Board board, const int maxDepth, bool maximizingPlayer, int alpha, int beta); // Function that starts the threading process
std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta); // Function that performs the minimax past the threading process
std::pair<int, int> quiescence(Board board, int alpha, int beta); // Implementation of quiescence search to explore the quiet moves
std::pair<bool, std::vector<int> > aboutToWin(Board board, char givenPlayer); // Checks if a given player has a winning move
std::vector<coordDirection> isGameDone(std::vector<std::vector<char> > &matrix, const char givenPlayer); // Checks if the given player has won
int getScore(Board board, const char givenPlayer); // Gets the calculated score of the board
int scoreBoard(Board board, const char givenPlayer, const char assignedPlayer); // Scores the current position of the board
std::vector<coordDirection> connectTwos(const std::vector<std::vector<char> > &board, const int rows, const int columns, const char givenPlayer); // Returns a vector of coordinates each describing the start of a connect two
std::vector<coordDirection> findConnectThrees(const std::vector<std::vector<char> > &board, const std::vector<coordDirection> &connectTwos, const int rows, const int columns, const char givenPlayer); // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
int countCenter(std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer); // Counts the number of pieces in the center of the board
int countEdges(std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer); // Counts the number of pieces in edge columns of the board
bool containedConnect(coordDirection connected, bool type, std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer); // Checks if the given connect two or three is blocked (pieces on either side of it)
std::vector<winInfo> possibleWin(const Board board, const int rows, const int columns, const char givenPlayer); // Finds a possible win
std::vector<coordDirection> findConnectFours(const std::vector<std::vector<char> > &board, const std::vector<coordDirection> &connectThrees, const int rows, const int columns, const char givenPlayer); // Finds wins in the board
bool canUpdateBoard(const std::string game, const int toUpdate); // Checks if you can add a number to a current game; prevents segmentation faults
#endif