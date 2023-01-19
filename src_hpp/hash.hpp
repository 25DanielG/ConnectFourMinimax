#ifndef HASH_HPP_
#define HASH_HPP_
#include <string>
#include <unordered_map>
void add(std::string board, int depth, int score, int move); // Adds the generated move to the transposition table
void load(std::unordered_map<std::string, std::pair<int, std::pair<int,int> > > &map, const std::string &filename); // Loads the unordered map from txt file using boost iarchive serialization
std::pair<int, std::pair<int, int> > search(const std::string &key); // Searches for a string in the table and returns the value (pair<int, int>)
std::unordered_map<std::string, std::pair<int, std::pair<int, int> > > &getTable(); // Returns a pointer to the transposition table
void save(std::unordered_map<std::string, std::pair<int, std::pair<int,int> > > &map, const std::string &filename); // Saves the map now using boost unordered map serialization
#endif