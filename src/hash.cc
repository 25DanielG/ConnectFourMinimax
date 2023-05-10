#include <iostream>
#include <fstream>
#include <unordered_map>
#include <boost/serialization/unordered_map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

std::unordered_map<std::string, std::pair<int, std::pair<int, int> > > transposition_table;

void add(std::string board, int depth, int score, int move) {
    auto it = transposition_table.find(board);
    if (it != transposition_table.end()) {
        if (depth > it->second.first) {
            it->second = std::make_pair(depth, std::make_pair(score, move));
        }
    } else {
        transposition_table[board] = std::make_pair(depth, std::make_pair(score, move));
    }
}

void save(std::unordered_map<std::string, std::pair<int, std::pair<int,int> > > &map, const std::string &filename) {
    std::ofstream ofs(filename);
    boost::archive::binary_oarchive oa(ofs);
    oa << transposition_table;
}

void load(std::unordered_map<std::string, std::pair<int, std::pair<int,int> > > &map, const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs.good()) {
        std::cerr << "File not found or empty" << std::endl;
        return;
    }
    try {
        boost::archive::binary_iarchive ia(ifs);
        ia >> map;
    } catch (boost::archive::archive_exception &e) {
        std::cerr << "Error reading file: " << e.what() << std::endl;
    }
    ifs.close();
}

std::pair<int, std::pair<int, int> > search(const std::string &key) {
    auto it = transposition_table.find(key);
    if (it != transposition_table.end()) {
        return it->second;
    } else {
        return std::make_pair(0, std::make_pair(0, 0));
    }
}

std::unordered_map<std::string, std::pair<int, std::pair<int, int> > > &getTable() {
    return transposition_table;
}