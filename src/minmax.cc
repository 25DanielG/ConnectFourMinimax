#include "../src_hpp/minmax.hpp"
#include "../src_hpp/board.hpp"
#include "../src_hpp/boardgui.hpp"
#include "../src_hpp/threader.hpp"
#include "../src_hpp/hash.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <iostream>
#include <queue>
#include <stdint.h>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;

const int NUM_THREADS = 8;
const std::string file_path = "transposition_table.bin";

std::pair<int, int> threading(Board board, int maxDepth, int alpha, int beta) {
    char computer = 'O';
    auto blockMove = aboutToWin(board, computer);
    if (blockMove.first) {
        return std::make_pair(INT32_MAX, blockMove.second[0]);
    }
    char player = 'X';
    blockMove = aboutToWin(board, player);
    if (blockMove.first) {
        return std::make_pair(INT32_MAX, blockMove.second[0]);
    }
    int bestScore = INT_MIN;
    int bestMove = NO_MOVE;
    // Iterative deepening
    for (int depth = 1; depth <= maxDepth; ++depth) {
        auto table = getTable();
        auto it = table.find(board.currentGame);
        if (it != table.end() && it->second.first >= depth) {
            bestScore = it->second.second.first;
            bestMove = it->second.second.second;
            return std::make_pair(bestScore, bestMove);
        }
        boost::thread_group threads;
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
            threads.create_thread(boost::bind(&minimax_thread, i));
        }
        threads.join_all();
        std::vector<std::pair<int, int> > *results = getResults();
        for (auto result : *results) {
            if (result.first >= max.first)
                max = result;
        }
        if(max.second == NO_MOVE) {
            cerr << "going to return -1 change to: " << results->at(0).second << endl;
            max = results->at(0);
        } else {
            add(board.currentGame, depth, max.first, max.second);
        }
        results->clear();
        bestScore = max.first;
        bestMove = max.second;
    }
    return std::make_pair(bestScore, bestMove);
}

std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta) {
    if (depth == 0 || board.currentGame.length() >= NUM_COLUMNS * NUM_ROWS) {
        // Quiescence
        int score = getScore(board, 'O');
        if (abs(score) < threshold) {
            return quiescence(board, alpha, beta);
        } else {
            return std::make_pair(score, NO_MOVE);
        }
    }
    auto table = getTable();
    auto it = table.find(board.currentGame);
    if (it != table.end() && it->second.first >= depth) {
        return std::make_pair(it->second.second.first, it->second.second.second);
    }
    Board updated = board;
    char player = maximizingPlayer ? 'X' : 'O';
    char opponent = maximizingPlayer ? 'O' : 'X';
    auto blockMove = aboutToWin(board, opponent);
    if (blockMove.first) {
        return std::make_pair(maximizingPlayer ? INT32_MAX : INT32_MIN, blockMove.second[0]);
    }
    std::pair<int, int> ret = maximizingPlayer ? std::make_pair(INT16_MIN, NO_MOVE) : std::make_pair(INT16_MAX, NO_MOVE);
    // Move ordering
    std::vector<std::pair<Board, std::pair<int, int> > > moves;
    for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
        if(!canUpdateBoard(board.currentGame, i)) continue;
        Board updated = board;
        updated.currentGame += (i + '0');
        moves.push_back(std::make_pair(updated, std::make_pair(getScore(updated, 'O'), i)));
    }
    std::sort(moves.begin(), moves.end(), sortFunc);
    for(auto move : moves) {
        int compValue = (minimax(move.first, depth - 1, !maximizingPlayer, alpha, beta)).first;
        if(maximizingPlayer) {
            if(compValue >= ret.first) {
                ret.first = compValue;
                ret.second = move.second.second;
            }
            alpha = std::max(alpha, ret.first);
        } else {
            if(compValue <= ret.first) {
                ret.first = compValue;
                ret.second = move.second.second;
            }
            beta = std::min(beta, ret.first);
        }
        if(alpha >= beta) break;
    }
    if(ret.second == -1) {
        add(board.currentGame, depth, ret.first, ret.second);
    }
    return ret;
}

std::pair<int, int> quiescence(Board board, int alpha, int beta) {
    int score = getScore(board, 'O');
    if (abs(score) < threshold) {
        return std::make_pair(score, NO_MOVE);
    }
    std::pair<int, int> best = std::make_pair(INT16_MIN, NO_MOVE);
    for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
        if (!canUpdateBoard(board.currentGame, i)) continue;
        Board updated = board;
        updated.currentGame += (i + '0');
        int value = quiescence(updated, alpha, beta).first;
        if (value > best.first) {
            best.first = value;
            best.second = i;
        }
        alpha = std::max(alpha, value);
        if (alpha >= beta) break;
    }
    return best;
}

void performMove(Board gameBoard) {
    gameBoard.computeBoard();
    std::pair<int, int> nextMove = threading(gameBoard, minimaxDepth, INT16_MIN, INT16_MAX);
    gameBoard.currentGame += std::to_string(nextMove.second);
    std::cerr << "Returned: " << nextMove.second << std::endl;
    std::vector<std::vector<char> > matrix = gameBoard.getMatrixBoard();
    updateBoard(gameBoard);
    cerr << endl << "SCORE: " << nextMove.first << endl << endl;
    if (isGameDone(matrix, 'X').size() > 0) {
        cout << "You Win!" << endl;
        save(getTable(), file_path);
        endGame();
        return;
    } else if (isGameDone(matrix, 'O').size() > 0) {
        cout << "You Lose!" << endl;
        save(getTable(), file_path);
        endGame();
        return;
    } else if (gameBoard.currentGame.length() >= 42) {
        cout << "Tie!" << endl;
        save(getTable(), file_path);
        endGame();
        return;
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
    // Calculate final score
    int score = 0, numEmpty = (NUM_COLUMNS * NUM_ROWS) - (board.currentGame.length());
    score += numInCenter * 2.5;
    score -= numInEdge;
    score += numConnectTwo * 5;
    score += numConnectThree * 20;
    // score += numEmpty * 0.1;
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

bool sortFunc(std::pair<Board, std::pair<int, int> > a, std::pair<Board, std::pair<int, int> > b) {
    return a.second.first > b.second.first;
}