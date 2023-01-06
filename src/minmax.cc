#include "../hsrc/minmax.hpp"
#include "../hsrc/board.hpp"
#include "../hsrc/boardgui.hpp"
#include <algorithm>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdint.h>
#include <string>
#include <vector>
#define minimaxDepth 7
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
int cnt = 0;
const int NUM_THREADS = 8;
std::queue<std::pair<Board, int> > jobQueue;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;
std::pair<int, int> result;

void *minimax_thread(void *arg) {
    while (true) {
        pthread_mutex_lock(&queueMutex);
        while (jobQueue.empty()) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }
        auto job = jobQueue.front();
        jobQueue.pop();
        pthread_mutex_unlock(&queueMutex);
        auto board = job.first;
        auto depth = job.second;
        if (depth == 0) {
            result = std::make_pair(getScore(board, 'X'), -1);
            return NULL;
        }
        Board updated = board;
        updated.currentGame += "9"; // Add the last character, 9 to throw segmentation fault if not overriden
        auto isWin = aboutToWin(board, 'O');
        if (isWin.first) {
            result = std::make_pair(INT32_MAX, isWin.second[0]);
            return NULL;
        }
        isWin = aboutToWin(board, 'X');
        if (isWin.first) {
            result = std::make_pair(INT32_MIN, isWin.second[0]);
            return NULL;
        }
        for (unsigned int i = 0; i < 7; ++i) {
            if (!canUpdateBoard(board.currentGame, i))
                continue;
            updated.currentGame[updated.currentGame.length() - 1] = i + '0'; // Override last character
            // jobQueue.push(std::make_pair(updated, depth - 1));
            addJob(std::make_pair(updated, depth - 1));
        }
        pthread_cond_broadcast(&queueCond);
    }
}

std::pair<int, int> minimax(Board board, const int depth, bool maximizingPlayer, int alpha, int beta) {
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, minimax_thread, NULL);
    }
    // jobQueue.push(std::make_pair(board, depth));
    addJob(std::make_pair(board, depth));
    pthread_cond_broadcast(&queueCond);
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    return result;
}

void addJob(std::pair<Board, int> job) {
    pthread_mutex_lock(&queueMutex);
    jobQueue.push(job);
    pthread_mutex_unlock(&queueMutex);
}

void performMove(Board gameBoard, bool computer) {
    gameBoard.computeBoard();
    cout << "Board:" << endl;
    gameBoard.printBoard();
    vector<vector<char> > matrix = gameBoard.getMatrixBoard();
    if (isGameDone(matrix, 'X').size() > 0) {
        cout << "You Win!" << endl;
        return;
    } else if (isGameDone(matrix, 'O').size() > 0) {
        cout << "You Lose!" << endl;
        return;
    }
    if (computer) {
        int nextMove = minimax(gameBoard, minimaxDepth, true, INT32_MIN, INT32_MAX).second;
        cerr << "Return of minimax: " << nextMove << endl;
        gameBoard.currentGame += std::to_string(nextMove);
        cerr << "Current game is: " << gameBoard.currentGame << endl;
    } else {
        cout << "Enter a column to move in: ";
        int columnMove;
        std::cin >> columnMove;
        gameBoard.currentGame += std::to_string(columnMove);
        cerr << "Current game is: " << gameBoard.currentGame << endl;
    }
    updateBoard(gameBoard);
}

std::pair<bool, vector<int> > aboutToWin(Board board, char givenPlayer) {
    vector<winInfo> wins = possibleWin(board, board.rows, board.columns, givenPlayer);
    vector<int> cols;
    for (winInfo w : wins)
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
    int numInEdge = countEdges(computedBoard, board.rows, board.columns, assignedPlayer);
    vector<winInfo> possibleWins = possibleWin(board, board.rows, board.columns, assignedPlayer);
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
        score += numPossibleWins * 100;
    return score;
}

vector<coordDirection> connectTwos(const vector<vector<char> > &board, const int rows, const int columns, const char givenPlayer) { // Returns a vector of coordinates each describing the start of a connect two
    vector<coordDirection> arrCoords;
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

vector<coordDirection> findConnectThrees(const vector<vector<char> > &board, const vector<coordDirection> &connectTwos, const int rows, const int columns, const char givenPlayer) { // Returns a vector of all the connect threes using the connect twos & directions found from the connect twos function
    vector<coordDirection> arrRet;
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

int countCenter(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Counts the number of pieces in the center of the board
    const int colToSearch = (columns / 2);
    int cnt = 0;
    for (int i = 0; i < rows; ++i) {
        if (board[i][colToSearch] == givenPlayer) {
            ++cnt;
        }
    }
    return cnt;
}

int countEdges(vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Counts the number of pieces in edge columns of the board
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

bool containedConnect(coordDirection connected, bool type, vector<vector<char> > board, const int rows, const int columns, const char givenPlayer) { // Checks if the given connect two or three is blocked (pieces on either side of it)
    if (type) {                                                                                                                                     // Connect two
        if (connected.direction == "down_left") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if ((((connected.coordinate.first == 0) || (connected.coordinate.second == columns - 1)) || ((board[connected.coordinate.first - 1][connected.coordinate.second + 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second + 1] != '#'))) && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second <= 1)) || ((board[connected.coordinate.first + 2][connected.coordinate.second - 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second - 2] != '#'))))
                return false;
        } else if (connected.direction == "down") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if ((((connected.coordinate.first == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second] != '#'))) && (((connected.coordinate.first >= rows - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second] != '#'))))
                return false;
        } else if (connected.direction == "down_right") {
            // IF UPPER BOUND OR UPPER HAS ENEMY PIECE    AND    LOWER BOUND OR HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if ((((connected.coordinate.first == 0) || (connected.coordinate.second == 0)) || ((board[connected.coordinate.first - 1][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first - 1][connected.coordinate.second - 1] != '#'))) && (((connected.coordinate.first >= rows - 2) || (connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first + 2][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first + 2][connected.coordinate.second + 2] != '#'))))
                return false;
        } else {
            // IF LEFT BOUND OR LEFT HAS ENEMY PIECE    AND    RIGHT BOUND OR RIGHT HAS ENEMY PIECE : RETURN FALSE (BOTH SIDES BLOCKED OR BOUNDED)
            if ((((connected.coordinate.second == 0)) || ((board[connected.coordinate.first][connected.coordinate.second - 1] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second - 1] != '#'))) && (((connected.coordinate.second >= columns - 2)) || ((board[connected.coordinate.first][connected.coordinate.second + 2] != givenPlayer) && (board[connected.coordinate.first][connected.coordinate.second + 2] != '#'))))
                return false;
        }
    } else { // Connect three
    }
    return true;
}

vector<winInfo> possibleWin(const Board board, const int rows, const int columns, const char givenPlayer) { // Finds a possible win
    vector<winInfo> arrWins;
    Board testBoard;
    for (int i = 0; i < board.columns; ++i) {
        if (!canUpdateBoard(board.currentGame, i))
            continue;
        testBoard = board;
        testBoard.currentGame += i + '0';
        auto matrix = testBoard.getMatrixBoard();
        vector<coordDirection> oneWins = isGameDone(matrix, givenPlayer);
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

vector<coordDirection> findConnectFours(const vector<vector<char> > &board, const vector<coordDirection> &connectThrees, const int rows, const int columns, const char givenPlayer) { // Finds wins in the board
    vector<coordDirection> wins;
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

bool canUpdateBoard(const string game, const int toUpdate) {
    char toChar = toUpdate + '0';
    int cnt = 0;
    for (int i = 0; i < game.length(); ++i) {
        if (game.at(i) == toChar)
            ++cnt;
    }
    return cnt < 6;
}