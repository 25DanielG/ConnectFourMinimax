#ifndef MINMAX_HPP_
#define MINMAX_HPP_
#include <vector>
#include "./board.hpp"
#include "./treeNode.hpp"
using std::vector;
struct coordDirection {
    std::pair<int, int> coordinate;
    std::string direction;
};
std::pair<int, int> minimax(treeNode<Board> &node, const int depth, bool maximizingPlayer, int bestValue); // Minimax function
void createTree(treeNode<Board> &node, const int depth); // Creates the tree containing all possible moves to a specified depth
int scoreBoard(Board board, const char givenPlayer); // Scores the current position of the board
vector<coordDirection> connectTwos(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Returns a vector of coordinates each describing the start of a connect two
vector<coordDirection> findConnectThrees(vector<vector<char> > board, vector<coordDirection> connectTwos, const int rows, const int columns, const char givenPlayer); // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
int countCenter(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Counts the number of pieces in the center of the board
bool containedConnect(coordDirection connected, bool type, vector<vector<char> > board, const int rows, const int columns, const char givenPlayer); // Checks if the given connect two or three is blocked (pieces on either side of it)
vector<coordDirection> possibleWin(vector<vector<char> > board, vector<coordDirection> connectThrees, const int rows, const int columns, const char givenPlayer); // Finds a possible win
#endif