#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "../hsrc/minmax.hpp"
#include "../hsrc/board.hpp"
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::cerr;
std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta) {
    if(depth == 0)
        return std::make_pair(getScore(board, 'X'), -1);
    Board updated = board;
    std::pair<int, int> ret;
    ret.second = -1;
    updated.currentGame += "9"; // Add the last character, 9 to throw segmentation fault if not overriden
    if(maximizingPlayer) {
        ret.first = INT32_MIN;
        auto isWin = aboutToWin(board, 'O');
        if(isWin.first) {
            ret.second = isWin.second[0];
            return ret;
        }
        isWin = aboutToWin(board, 'X');
        if(isWin.first) {
            ret.second = isWin.second[0];
            return ret;
        }
        for(unsigned int i = 0; i < 7; ++i) {
            if(!canUpdateBoard(board.currentGame, i)) continue;
            updated.currentGame[updated.currentGame.length() - 1] = i + '0'; // Override last character
            int compValue = (minimax(updated, depth - 1, false, alpha, beta)).first;
            if(compValue >= ret.first) {
                ret.first = compValue;
                ret.second = i;
            }
            alpha = std::max(alpha, ret.first);
            if(alpha >= beta) break;
        }
    } else {
        ret.first = INT32_MAX;
        auto isWin = aboutToWin(board, 'X');
        if(isWin.first) {
            ret.second = isWin.second[0];
            return ret;
        }
        isWin = aboutToWin(board, 'O');
        if(isWin.first) {
            ret.second = isWin.second[0];
            return ret;
        }
        for(unsigned int i = 0; i < 7; ++i) {
            if(!canUpdateBoard(board.currentGame, i)) continue;
            updated.currentGame[updated.currentGame.length() - 1] = i + '0'; // Override last character
            int compValue = (minimax(updated, depth - 1, true, alpha, beta)).first;
            if(compValue <= ret.first) {
                ret.first = compValue;
                ret.second = i;
            }
            beta = std::min(beta, ret.first);
            if(alpha >= beta) break;
        }
    }
    return ret;
}
std::pair<bool, vector<int> > aboutToWin(Board board, char givenPlayer) {
    vector<vector<char> > matrix = board.getMatrixBoard();
    vector<winInfo> wins = possibleWin(matrix, board.rows, board.columns, givenPlayer);
    vector<int> cols;
    for(winInfo w : wins)
        cols.push_back(w.winCol);
    return std::make_pair(wins.size() > 0, cols);
}
vector<coordDirection> isGameDone(vector<vector<char> > &matrix, const char givenPlayer) {
    int rows = matrix.size(), columns = matrix[0].size();
    vector<coordDirection> twos = connectTwos(matrix, rows, columns, givenPlayer);
    vector<coordDirection> threes = findConnectThrees(matrix, twos, rows, columns, givenPlayer);
    vector<coordDirection> wins = findConnectFours(matrix, threes, rows, columns, givenPlayer);
    return wins;
}
int getScore(Board board, const char givenPlayer) {
    char oppPlayer = givenPlayer == 'X' ? 'O' : 'X';
    int fScore = scoreBoard(board, givenPlayer, givenPlayer);
    int sScore = scoreBoard(board, givenPlayer, oppPlayer);
    return fScore - sScore;
}
int scoreBoard(Board board, const char givenPlayer, const char assignedPlayer) { // Scores the current position of the board
    vector<vector<char> > computedBoard = board.getMatrixBoard();
    vector<coordDirection> arrConnectTwos = connectTwos(computedBoard, board.rows, board.columns, assignedPlayer);
    int numConnectTwo = arrConnectTwos.size();
    vector<coordDirection> arrConnectThrees = findConnectThrees(computedBoard, arrConnectTwos, board.rows, board.columns, assignedPlayer);
    int numConnectThree = arrConnectThrees.size();
    int numInCenter = countCenter(computedBoard, board.rows, board.columns, assignedPlayer);
    vector<winInfo> possibleWins = possibleWin(computedBoard, board.rows, board.columns, assignedPlayer);
    int numPossibleWins = possibleWins.size();
    // cerr << "ConnectTwos: " << numConnectTwo << " ConnectThrees: " << numConnectThree << " CenterPieces: " << numInCenter << " Possible wins: " << numPossibleWins << endl;
    // Calculate final score
    int score = 0;
    score += numInCenter;
    score += numConnectTwo * 2;
    score += numConnectThree * 4;
    if(givenPlayer == assignedPlayer)
        score += numPossibleWins * 10000;
    else
        score += numPossibleWins * 100;
    return score;
}
vector<coordDirection> connectTwos(const vector<vector<char> > &board, const int rows, const int columns, const char givenPlayer) { // Returns a vector of coordinates each describing the start of a connect two
    vector<coordDirection> arrCoords;
    coordDirection tmpRet;
    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < columns; ++j) {
            if(board[i][j] != givenPlayer) { // Current square not filled in
                continue;
            }
            if(i < rows - 1) { // !Bottom
                if(j == 0) { // Left
                    if(board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i + 1][j + 1]) { // Bottom right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_right";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                } else if(j == columns - 1) { // Right
                    if(board[i][j] == board[i + 1][j - 1]) { // Bottom left
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_left";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                } else { // Mid
                    if(board[i][j] == board[i + 1][j - 1]) { // Bottom left
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_left";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i + 1][j + 1]) { // Bottom right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_right";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if(board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                }
            } else { // Bottom
                if(!(j == rows - 1)) { // Left || Mid
                    if(board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if(containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                }
            }
        }
    }
    return arrCoords;
}
vector<coordDirection> findConnectThrees(const vector<vector<char> > &board, const vector<coordDirection> &connectTwos, const int rows, const int columns, const char givenPlayer) { // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
    vector<coordDirection> arrRet;
    std::pair<int, int> tmpCoord;
    for(coordDirection singleCD : connectTwos) {
        tmpCoord = singleCD.coordinate;
        if(singleCD.direction == "down_left") {
            ++tmpCoord.first; --tmpCoord.second;
            if((tmpCoord.first == rows - 1) || (tmpCoord.second == 0))
                continue;
            ++tmpCoord.first; --tmpCoord.second;
            if(board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else if(singleCD.direction == "down") {
            ++tmpCoord.first;
            if(tmpCoord.first == rows - 1)
                continue;
            ++tmpCoord.first;
            if(board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else if(singleCD.direction == "down_right") {
            ++tmpCoord.first; ++tmpCoord.second;
            if((tmpCoord.first == rows - 1) || (tmpCoord.second == columns - 1))
                continue;
            ++tmpCoord.first; ++tmpCoord.second;
            if(board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else {
            ++tmpCoord.second;
            if(tmpCoord.second == columns - 1) {
                continue;
            }
            ++tmpCoord.second;
            if(board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        }
    }
    return arrRet;
}
int countCenter(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Counts the number of pieces in the center of the board
    const int colToSearch = (columns / 2);
    int cnt = 0;
    for(int i = 0; i < rows; ++i) {
        if(board[i][colToSearch] == givenPlayer) {
            ++cnt;
        }
    }
    return cnt;
}
bool containedConnect(coordDirection connected, bool type, vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Checks if the given connect two or three is blocked (pieces on either side of it)
    if(type) { // Connect two
        if(connected.direction == "down_left") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if((((connected.coordinate.first == 0) || (connected.coordinate.second == columns - 1)) || ((board[connected.coordinate.first - 1][connected.coordinate.second + 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second + 1] != '#'))) 
             && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second <= 1)) || ((board[connected.coordinate.first + 2][connected.coordinate.second - 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second - 2] != '#')))) return false;
        } else if(connected.direction == "down") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if((((connected.coordinate.first == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second] != '#'))) 
             && (((connected.coordinate.first >= rows - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second] != '#')))) return false;
        } else if(connected.direction == "down_right") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if((((connected.coordinate.first == 0) || (connected.coordinate.second == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second - 1] != '#'))) 
             && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second + 2] != '#')))) return false;
        } else {
            // IF LEFT BOUND OR LEFT HAS ENEMY PIECE    AND    RIGHT BOUND OR RIGHT HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if((((connected.coordinate.second == 0)) || ((board[connected.coordinate.first][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second - 1] != '#'))) 
             && (((connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second + 2] != '#')))) return false;
        }
    } else { // Connect three

    }
    return true;
}
vector<winInfo> possibleWin(const vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Finds a possible win
    vector<winInfo> arrWins;
    vector<vector<char> > testBoard = board;
    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < columns; ++j) {
            testBoard = board;
            if(testBoard[i][j] != '#' || (i != rows - 1 && testBoard[i + 1][j] == '#')) continue;
            testBoard[i][j] = givenPlayer;
            vector<coordDirection> oneWins = isGameDone(testBoard, givenPlayer);
            if(oneWins.size() > 0) {
                for(coordDirection x : oneWins) {
                    winInfo ret;
                    ret.coords = x;
                    ret.winCol = j;
                    arrWins.push_back(ret);
                }
            }
        }
    }
    return arrWins;
}
vector<coordDirection> findConnectFours(const vector<vector<char> > &board, const vector<coordDirection> &connectThrees, const int rows, const int columns, const char givenPlayer) { // Finds wins in the board
    vector<coordDirection> wins;
    for(coordDirection singleCD : connectThrees) {
        if(singleCD.direction == "down_left") {
            /*if(singleCD.coordinate.first == 0 || singleCD.coordinate.second == columns - 1 // Origin bound
                || board[singleCD.coordinate.first - 1][singleCD.coordinate.second + 1] != givenPlayer) {
                continue;
            }*/
            if (singleCD.coordinate.first >= rows - 3 || singleCD.coordinate.second <= 2) { // Extreme bound
                continue;
            }
            /*if(board[singleCD.coordinate.first - 1][singleCD.coordinate.second + 1] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first - 1, singleCD.coordinate.second + 1);
                wins.push_back(ret);
            }*/
            if(board[singleCD.coordinate.first + 3][singleCD.coordinate.second - 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else if(singleCD.direction == "down") {
            /*if(singleCD.coordinate.first == 0) { // Origin bound (no extreme bound)
                continue;
            }
            if(board[singleCD.coordinate.first - 1][singleCD.coordinate.second] == '#') {
                ret.winCol = singleCD.coordinate.second;
                arrWins.push_back(ret);
            }*/
            if(singleCD.coordinate.first > rows - 4) {
                continue;
            }
            if(board[singleCD.coordinate.first + 3][singleCD.coordinate.second] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else if(singleCD.direction == "down_right") {
            /*if(singleCD.coordinate.first == 0 || singleCD.coordinate.second == 0 // Origin bound
                || board[singleCD.coordinate.first - 1][singleCD.coordinate.second - 1] != '#') {
                continue;
            }*/
            if(singleCD.coordinate.first >= rows - 3 || singleCD.coordinate.second >= columns - 3) { // Extreme bound
                continue;
            }
            /*if(checkOrigin) {
                if(board[singleCD.coordinate.first][singleCD.coordinate.second - 1] != '#') {
                    ret.winCol = singleCD.coordinate.second - 1;
                    arrWins.push_back(ret);
                }
            }*/
            if(board[singleCD.coordinate.first + 3][singleCD.coordinate.second + 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else { // Right
            /*if(singleCD.coordinate.second == 0 || board[singleCD.coordinate.first][singleCD.coordinate.second - 1] != '#') { // Origin bounds
                checkOrigin = false;
            }*/
            if(singleCD.coordinate.second >= columns - 3) { // Extreme bounds
                continue;
            }
            /*if(checkOrigin) {
                if(singleCD.coordinate.first == rows - 1 || board[singleCD.coordinate.first + 1][singleCD.coordinate.second - 1] != '#') {
                    ret.winCol = singleCD.coordinate.second - 1;
                    arrWins.push_back(ret);
                }
            }*/
            if(board[singleCD.coordinate.first][singleCD.coordinate.second + 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        }
    }
    return wins;
}
bool canUpdateBoard(const string game, const int toUpdate) {
    char toChar = toUpdate + '0';
    int cnt = 0;
    for(int i = 0; i < game.length(); ++i) {
        if(game.at(i) == toChar)
            ++cnt;
    }
    return cnt < 6;
}