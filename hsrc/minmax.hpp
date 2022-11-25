#ifndef MINMAX_HPP_
#define MINMAX_HPP_
#include <vector>
#include "./board.hpp"
using std::vector;
using std::string;
struct coordDirection {
    std::pair<int, int> coordinate;
    std::string direction;
};
struct winInfo {
    coordDirection coords;
    int winCol;
};
std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta); // Minimax function
std::pair<bool, vector<int> > aboutToWin(Board board, char givenPlayer); // Checks if a given player has a winning move
vector<coordDirection> isGameDone(vector<vector<char> > &matrix, const char givenPlayer); // Checks if the given player has won
int getScore(Board board, const char givenPlayer); // Gets the calculated score of the board
int scoreBoard(Board board, const char givenPlayer, const char assignedPlayer); // Scores the current position of the board
vector<coordDirection> connectTwos(const vector<vector<char> > &board, const int rows, const int columns, const char givenPlayer); // Returns a vector of coordinates each describing the start of a connect two
vector<coordDirection> findConnectThrees(const vector<vector<char> > &board, const vector<coordDirection> &connectTwos, const int rows, const int columns, const char givenPlayer); // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
int countCenter(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Counts the number of pieces in the center of the board
bool containedConnect(coordDirection connected, bool type, vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Checks if the given connect two or three is blocked (pieces on either side of it)
vector<winInfo> possibleWin(const vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Finds a possible win
vector<coordDirection> findConnectFours(const vector<vector<char> > &board, const vector<coordDirection> &connectThrees, const int rows, const int columns, const char givenPlayer); // Finds wins in the board
bool canUpdateBoard(const string game, const int toUpdate); // Checks if you can add a number to a current game; prevents segmentation faults
#endif