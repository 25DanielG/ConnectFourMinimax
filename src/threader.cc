#include <pthread.h>
#include <iostream>
#include <queue>
#include "../src_hpp/minmax.hpp"
using std::cerr;

std::queue<minimaxValues> jobQueue;
std::vector<std::pair<int, int> > results;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resultsCond = PTHREAD_COND_INITIALIZER;

void *addJob(minimaxValues job) {
    pthread_mutex_lock(&queueMutex);
    cerr << "addJob queue is now size of: " << jobQueue.size() << std::endl;
    jobQueue.push(job);
    pthread_mutex_unlock(&queueMutex);
    return NULL;
}

void *addResult(std::pair<int, int> res) {
    pthread_mutex_lock(&resultsMutex);
    results.push_back(res);
    pthread_mutex_unlock(&resultsMutex);
    return NULL;
}

void *minimax_thread(void *arg) {
    while(true) {
        pthread_mutex_lock(&queueMutex);
        if(jobQueue.empty()) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        cerr << "thread #" << arg << " took a job now job queue is of size " << jobQueue.size() << std::endl;
        auto job = jobQueue.front();
        jobQueue.pop();
        pthread_mutex_unlock(&queueMutex);
        Board board = job.board;
        int depth = job.depth;
        bool maximizingPlayer = job.maximizingPlayer;
        int alpha = job.alpha;
        int beta = job.beta;

        int maxVal = maximizingPlayer ? INT32_MIN : INT32_MAX;
        std::pair<int, int> max = std::make_pair(maxVal, -1);
        Board updated = board;
        updated.currentGame += "9";
        for (unsigned int i = 0; i < NUM_COLUMNS; ++i) {
            if(!canUpdateBoard(board.currentGame, i)) continue;
            updated.currentGame[updated.currentGame.length() - 1] = i + '0'; // Override last character
            std::pair<int, int> compVal = minimax(updated, depth - 1, !maximizingPlayer, alpha, beta);
            if (compVal.first < max.first) {
                max.first = compVal.first;
                max.second = i;
            }
        }
        addResult(max);
    }
}

std::vector<std::pair<int, int> > getResults() {
    return results;
}