#ifndef THREADER_HPP
#define THREADER_HPP
#include "./minmax.hpp"
void* minimax_thread(void *arg); // Threading function: takes jobs from job queue
void *addJob(minimaxValues job); // Adds a job to to job queue while mutex lock is held
void printJobs(); // Prints all the jobs in the queue while holding a mutex lock just in case
void *addResult(std::pair<int, int> res); // Sets the global result variable in a mutex lock
std::vector<std::pair<int, int> > *getResults(); // Returns the results vector
#endif