#include "../src_hpp/minmax.hpp"
#include "../src_hpp/board.hpp"
#include "../src_hpp/boardgui.hpp"
#include "../src_hpp/threader.hpp"
#include "../src_hpp/hash.hpp"
#include <algorithm>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdint.h>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;

pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;
const int NUM_THREADS = 8;
bool train = false;
const std::string file_path = "transposition_table.bin";

std::pair<int, int> threading(Board board, int depth, int alpha, int beta) {
    if(!train) {
        auto table = getTable();
        auto it = table->find(board.currentGame);
        if (it != table->end() && it->second.first >= depth) {
            return std::make_pair(it->second.second.first, it->second.second.second);
        }
    }
    pthread_t threads[NUM_THREADS];
    char computer = 'O';
    auto blockMove = aboutToWin(board, computer);
    if (blockMove.first) {
        return std::make_pair(INT32_MAX, blockMove.second[0]);
    }
    std::pair<int, int> max = std::make_pair(INT16_MIN, NO_MOVE);
    minimaxValues job = {board, depth - 1, false, alpha, beta, NO_MOVE};
    for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
        if(!canUpdateBoard(board.currentGame, i)) continue;
        Board updated = board;
        updated.currentGame += (i + '0');
        job.board = updated;
        job.move = i;
        addJob(job);
    }
    for (unsigned int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, minimax_thread, (void *) i);
    }
    pthread_cond_broadcast(&queueCond);
    for (unsigned int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    std::vector<std::pair<int, int> > *results = getResults();
    for (auto result : *results) {
        if (result.first >= max.first)
            max = result;
    }
    if(max.second == -1) {
        max = std::make_pair(0, results->at(0).second);
    } else {
        add(board.currentGame, depth, max.first, max.second);
    }
    results->clear();
    return max;
}

std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta) {
    if (depth == 0) {
        return std::make_pair(getScore(board, 'O'), NO_MOVE);
    }
    Board updated = board;
    char player = maximizingPlayer ? 'X' : 'O';
    char opponent = maximizingPlayer ? 'O' : 'X';
    auto blockMove = aboutToWin(board, opponent);
    if (blockMove.first) {
        return std::make_pair(maximizingPlayer ? INT32_MAX : INT32_MIN, blockMove.second[0]);
    }
    std::pair<int, int> ret = maximizingPlayer ? std::make_pair(INT16_MIN, NO_MOVE) : std::make_pair(INT16_MAX, NO_MOVE);
    updated.currentGame += "9"; // Add the last character, 9 to throw segmentation fault if not overriden
    for(unsigned int i = 0; i < NUM_COLUMNS; ++i) {
        if(!canUpdateBoard(board.currentGame, i)) continue;
        updated.currentGame[updated.currentGame.length() - 1] = i + '0'; // Override last character
        int compValue = (minimax(updated, depth - 1, !maximizingPlayer, alpha, beta)).first;
        if(maximizingPlayer) {
            if(compValue >= ret.first) {
                ret.first = compValue;
                ret.second = i;
            }
            alpha = std::max(alpha, ret.first);
        } else {
            if(compValue <= ret.first) {
                ret.first = compValue;
                ret.second = i;
            }
            beta = std::min(beta, ret.first);
        }
        if(alpha >= beta) break;
    }
    return ret;
}

void performMove(Board gameBoard) {
    gameBoard.computeBoard();
    std::pair<int, int> nextMove = threading(gameBoard, minimaxDepth, INT16_MIN, INT16_MAX);
    gameBoard.currentGame += std::to_string(nextMove.second);
    std::vector<std::vector<char> > matrix = gameBoard.getMatrixBoard();
    updateBoard(gameBoard);
    cerr << endl << nextMove.first << endl;
    if (isGameDone(matrix, 'X').size() > 0) {
        cout << "You Win!" << endl;
        save(file_path);
        return;
    } else if (isGameDone(matrix, 'O').size() > 0) {
        cout << "You Lose!" << endl;
        save(file_path);
        return;
    } else if (gameBoard.currentGame.length() >= 42) {
        cout << "Tie!" << endl;
        save(file_path);
        return;
    }
}

void trainComputer(int train_depth) {
    train = true;
    Board game;
    while (true) {
        game.computeBoard();
        game.printBoard();
        std::vector<std::vector<char> > matrix = game.getMatrixBoard();
        if (isGameDone(matrix, 'X').size() > 0 || isGameDone(matrix, 'O').size() > 0) {
            cout << "Game Ended" << endl;
            save(file_path);
            return;
        }
        int nextMove = threading(game, train_depth, INT16_MIN, INT16_MAX).second;
        game.currentGame += std::to_string(nextMove);
    }
}

std::pair<bool, std::vector<int> > aboutToWin(Board board, char givenPlayer) {
    std::vector<winInfo> wins = possibleWin(board, board.rows, board.columns, givenPlayer);
    std::vector<int> cols;
    for (winInfo w : wins)
        cols.push_back(w.winCol);
    return std::make_pair(wins.size() > 0, cols);
}

std::vector<coordDirection> isGameDone(std::vector<std::vector<char> > &matrix, const char givenPlayer) {
    int rows = matrix.size(), columns = matrix[0].size();
    std::vector<coordDirection> twos = connectTwos(matrix, rows, columns, givenPlayer);
    std::vector<coordDirection> threes = findConnectThrees(matrix, twos, rows, columns, givenPlayer);
    std::vector<coordDirection> wins = findConnectFours(matrix, threes, rows, columns, givenPlayer);
    return wins;
}

int getScore(Board board, const char givenPlayer) {
    char oppPlayer = givenPlayer == 'X' ? 'O' : 'X';
    int fScore = scoreBoard(board, givenPlayer, givenPlayer);
    int sScore = scoreBoard(board, givenPlayer, oppPlayer);
    return fScore - sScore;
}

int scoreBoard(Board board, const char givenPlayer, const char assignedPlayer) { // Scores the current position of the board
    std::vector<std::vector<char> > computedBoard = board.getMatrixBoard();
    std::vector<coordDirection> arrConnectTwos = connectTwos(computedBoard, board.rows, board.columns, assignedPlayer);
    int numConnectTwo = arrConnectTwos.size();
    std::vector<coordDirection> arrConnectThrees = findConnectThrees(computedBoard, arrConnectTwos, board.rows, board.columns, assignedPlayer);
    int numConnectThree = arrConnectThrees.size();
    int numInCenter = countCenter(computedBoard, board.rows, board.columns, assignedPlayer);
    int numInEdge = countEdges(computedBoard, board.rows, board.columns, assignedPlayer);
    std::vector<winInfo> possibleWins = possibleWin(board, board.rows, board.columns, assignedPlayer);
    int numPossibleWins = possibleWins.size();
    // cerr << "ConnectTwos: " << numConnectTwo << " ConnectThrees: " << numConnectThree << " CenterPieces: " << numInCenter << " Possible wins: " << numPossibleWins << endl;
    // Calculate final score
    int score = 0;
    score += numInCenter;
    score -= numInEdge * 3;
    score += numConnectTwo * 2;
    score += numConnectThree * 4;
    if (givenPlayer == assignedPlayer)
        score += numPossibleWins * 10000;
    else
        score += numPossibleWins * 1000;
    return score;
}

std::vector<coordDirection> connectTwos(const std::vector<std::vector<char> > &board, const int rows, const int columns, const char givenPlayer) { // Returns a vector of coordinates each describing the start of a connect two
    std::vector<coordDirection> arrCoords;
    coordDirection tmpRet;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (board[i][j] != givenPlayer) { // Current square not filled in
                continue;
            }
            if (i < rows - 1) {                           // !Bottom
                if (j == 0) {                             // Left
                    if (board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i + 1][j + 1]) { // Bottom right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_right";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                } else if (j == columns - 1) {                // Right
                    if (board[i][j] == board[i + 1][j - 1]) { // Bottom left
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_left";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                } else {                                      // Mid
                    if (board[i][j] == board[i + 1][j - 1]) { // Bottom left
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_left";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i + 1][j]) { // Bottom
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i + 1][j + 1]) { // Bottom right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "down_right";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                    if (board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                }
            } else {                                      // Bottom
                if (!(j == rows - 1)) {                   // Left || Mid
                    if (board[i][j] == board[i][j + 1]) { // Right
                        tmpRet.coordinate.first = i, tmpRet.coordinate.second = j;
                        tmpRet.direction = "right";
                        if (containedConnect(tmpRet, true, board, rows, columns, givenPlayer))
                            arrCoords.push_back(tmpRet);
                    }
                }
            }
        }
    }
    return arrCoords;
}

std::vector<coordDirection> findConnectThrees(const std::vector<std::vector<char> > &board, const std::vector<coordDirection> &connectTwos, const int rows, const int columns, const char givenPlayer) { // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
    std::vector<coordDirection> arrRet;
    std::pair<int, int> tmpCoord;
    for (coordDirection singleCD : connectTwos) {
        tmpCoord = singleCD.coordinate;
        if (singleCD.direction == "down_left") {
            ++tmpCoord.first;
            --tmpCoord.second;
            if ((tmpCoord.first == rows - 1) || (tmpCoord.second == 0))
                continue;
            ++tmpCoord.first;
            --tmpCoord.second;
            if (board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else if (singleCD.direction == "down") {
            ++tmpCoord.first;
            if (tmpCoord.first == rows - 1)
                continue;
            ++tmpCoord.first;
            if (board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else if (singleCD.direction == "down_right") {
            ++tmpCoord.first;
            ++tmpCoord.second;
            if ((tmpCoord.first == rows - 1) || (tmpCoord.second == columns - 1))
                continue;
            ++tmpCoord.first;
            ++tmpCoord.second;
            if (board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        } else if (singleCD.direction == "right") {
            ++tmpCoord.second;
            if (tmpCoord.second == columns - 1) {
                continue;
            }
            ++tmpCoord.second;
            if (board[tmpCoord.first][tmpCoord.second] == givenPlayer) {
                arrRet.push_back(singleCD);
            }
        }
    }
    return arrRet;
}

int countCenter(std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Counts the number of pieces in the center of the board
    const int colToSearch = (columns / 2);
    int cnt = 0;
    for (int i = 0; i < rows; ++i) {
        if (board[i][colToSearch] == givenPlayer) {
            ++cnt;
        }
    }
    return cnt;
}

int countEdges(std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Counts the number of pieces in edge columns of the board
    const int colToSearch = 0;
    const int colToSearchTwo = 6;
    int cnt = 0;
    for (int i = 0; i < rows; ++i) {
        if (board[i][colToSearch] == givenPlayer) {
            ++cnt;
        }
    }
    for (int i = 0; i < rows; ++i) {
        if (board[i][colToSearchTwo] == givenPlayer) {
            ++cnt;
        }
    }
    return cnt;
}

bool containedConnect(coordDirection connected, bool type, std::vector<std::vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Checks if the given connect two or three is blocked (pieces on either side of it)
    if (type) {                                                                                                                                               // Connect two
        if (connected.direction == "down_left") {
            if ((((connected.coordinate.first == 0) || (connected.coordinate.second == columns - 1)) || ((board[connected.coordinate.first - 1][connected.coordinate.second + 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second + 1] != '#'))) && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second <= 1)) || ((board[connected.coordinate.first + 2][connected.coordinate.second - 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second - 2] != '#'))))
                return false;
        } else if (connected.direction == "down") {
            if ((((connected.coordinate.first == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second] != '#'))) && (((connected.coordinate.first >= rows - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second] != '#'))))
                return false;
        } else if (connected.direction == "down_right") {
            if ((((connected.coordinate.first == 0) || (connected.coordinate.second == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second - 1] != '#'))) && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second + 2] != '#'))))
                return false;
        } else {
            if ((((connected.coordinate.second == 0)) || ((board[connected.coordinate.first][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second - 1] != '#'))) && (((connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second + 2] != '#'))))
                return false;
        }
    } else { // Connect three
    }
    return true;
}

std::vector<winInfo> possibleWin(const Board board, const int rows, const int columns, const char givenPlayer) { // Finds a possible win
    std::vector<winInfo> arrWins;
    Board testBoard;
    for (int i = 0; i < board.columns; ++i) {
        if (!canUpdateBoard(board.currentGame, i))
            continue;
        testBoard = board;
        testBoard.currentGame += i + '0';
        auto matrix = testBoard.getMatrixBoard();
        std::vector<coordDirection> oneWins = isGameDone(matrix, givenPlayer);
        if (oneWins.size() > 0) {
            for (coordDirection x : oneWins) {
                winInfo ret;
                ret.coords = x;
                ret.winCol = i;
                arrWins.push_back(ret);
            }
        }
    }
    return arrWins;
}

std::vector<coordDirection> findConnectFours(const std::vector<std::vector<char> > &board, const std::vector<coordDirection> &connectThrees, const int rows, const int columns, const char givenPlayer) { // Finds wins in the board
    std::vector<coordDirection> wins;
    for (coordDirection singleCD : connectThrees) {
        if (singleCD.direction == "down_left") {
            if (singleCD.coordinate.first >= rows - 3 || singleCD.coordinate.second <= 2) { // Extreme bound
                continue;
            }
            if (board[singleCD.coordinate.first + 3][singleCD.coordinate.second - 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else if (singleCD.direction == "down") {
            if (singleCD.coordinate.first > rows - 4) {
                continue;
            }
            if (board[singleCD.coordinate.first + 3][singleCD.coordinate.second] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else if (singleCD.direction == "down_right") {
            if (singleCD.coordinate.first >= rows - 3 || singleCD.coordinate.second >= columns - 3) { // Extreme bound
                continue;
            }
            if (board[singleCD.coordinate.first + 3][singleCD.coordinate.second + 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        } else {                                             // Right
            if (singleCD.coordinate.second >= columns - 3) { // Extreme bounds
                continue;
            }
            if (board[singleCD.coordinate.first][singleCD.coordinate.second + 3] == givenPlayer) {
                coordDirection ret = singleCD;
                ret.coordinate = std::make_pair(singleCD.coordinate.first, singleCD.coordinate.second);
                wins.push_back(ret);
            }
        }
    }
    return wins;
}

bool canUpdateBoard(const std::string game, const int toUpdate) {
    char toChar = toUpdate + '0';
    int cnt = 0;
    for (int i = 0; i < game.length(); ++i) {
        if (game.at(i) == toChar)
            ++cnt;
    }
    return cnt < 6;
}