#ifndef HASH_HPP_
#define HASH_HPP_
#include <string>
#include <unordered_map>
void add_to_table(std::string board, int depth, int move); // Adds the generated move to the transposition table
void save(const std::string& file_path); // Saves the transposition table to a file
void load(const std::string &file_path); // Loads the transposition table back from the file
std::pair<int, int> search(const std::string &key); // Searches for a string in the table and returns the value (pair<int, int>)
std::unordered_map<std::string, std::pair<int, int> > *getTable(); // Returns a pointer to the transposition table
#endif