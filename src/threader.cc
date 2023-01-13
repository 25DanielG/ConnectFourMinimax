#include <pthread.h>
#include <iostream>
#include <queue>
#include <chrono>
#include "../src_hpp/minmax.hpp"
using namespace std::chrono;
using std::cerr;

std::queue<minimaxValues> jobQueue;
std::vector<std::pair<int, int> > results;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resultsCond = PTHREAD_COND_INITIALIZER;

void *addJob(minimaxValues job) {
    pthread_mutex_lock(&queueMutex);
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
        cerr << "Thread #" << arg << " took a job" << std::endl;
        auto start = high_resolution_clock::now();
        auto job = jobQueue.front();
        jobQueue.pop();
        pthread_mutex_unlock(&queueMutex);

        std::pair<int, int> max = minimax(job.board, job.depth, job.maximizingPlayer, job.alpha, job.beta);

        addResult(max);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        cerr << "Thread #" << arg << " finished job in: " << duration.count() << " milliseconds." << std::endl;
    }
}

std::vector<std::pair<int, int> > getResults() {
    return results;
}

void clearResults() {
    results.clear();
}