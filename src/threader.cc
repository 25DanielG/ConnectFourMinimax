#include <boost/thread.hpp>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <chrono>
#include "../src_hpp/minmax.hpp"
using namespace std::chrono;
using std::cerr;

std::queue<minimaxValues> jobQueue;
std::vector<std::pair<int, int> > results;
boost::mutex queueMutex, resultsMutex, coutMutex;

void *addJob(minimaxValues job) {
    queueMutex.lock();
    jobQueue.push(job);
    queueMutex.unlock();
    return NULL;
}

void printJobs() {
    queueMutex.lock();
    for(unsigned int i = 0; i < jobQueue.size(); ++i) {
        auto job = jobQueue.front();
        jobQueue.pop();
        jobQueue.push(job);
        cerr << job.board.currentGame << " ";
    }
    cerr << std::endl;
    queueMutex.unlock();
}

void *addResult(std::pair<int, int> res) {
    resultsMutex.lock();
    results.push_back(res);
    resultsMutex.unlock();
    return NULL;
}

void *minimax_thread(unsigned int arg) {
    while(true) {
        queueMutex.lock();
        if(jobQueue.empty()) {
            queueMutex.unlock();
            break;
        }
        auto start = high_resolution_clock::now();
        auto job = jobQueue.front();
        jobQueue.pop();
        queueMutex.unlock();

        addResult(std::make_pair(minimax(job.board, job.depth, job.maximizingPlayer, job.alpha, job.beta).first, job.move));

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        coutMutex.lock();
        cerr << "Thread #" << arg << " finished job in: " << duration.count() << " milliseconds." << std::endl;
        coutMutex.unlock();
    }
}

std::vector<std::pair<int, int> > *getResults() {
    auto *p = &results;
    return p;
}