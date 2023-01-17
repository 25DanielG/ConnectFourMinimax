#include <iostream>
#include <fstream>
#include <unordered_map>

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

void save(const std::string &file_path) {
    std::ofstream out(file_path, std::ios::binary);
    uint64_t size = transposition_table.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(size));
    for (const auto &[key, value] : transposition_table) {
        uint64_t key_size = key.size();
        out.write(reinterpret_cast<char *>(&key_size), sizeof(key_size));
        out.write(key.data(), key_size);
        out.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }
    out.close();
}

void load(const std::string &file_path) {
    std::ifstream in(file_path, std::ios::binary);
    if (!in) {
        std::cerr << "Error: File " << file_path << " does not exist." << std::endl;
        return;
    }
    uint64_t size;
    if (!in.read(reinterpret_cast<char *>(&size), sizeof(size)) || !in) {
        std::cerr << "Error: File " << file_path << " is empty." << std::endl;
        in.close();
        return;
    }
    for (uint64_t i = 0; i < size; ++i) {
        uint64_t key_size;
        if (!in.read(reinterpret_cast<char *>(&key_size), sizeof(key_size)) || !in) {
            std::cerr << "Error: File " << file_path << " is corrupted." << std::endl;
            in.close();
            return;
        }
        std::string key(key_size, ' ');
        if (!in.read(const_cast<char *>(key.c_str()), key_size) || !in) {
            std::cerr << "Error: File " << file_path << " is corrupted." << std::endl;
            in.close();
            return;
        }
        std::pair<int, std::pair<int, int> > value;
        if (!in.read(reinterpret_cast<char *>(&value), sizeof(value)) || !in) {
            std::cerr << "Error: File " << file_path << " is corrupted." << std::endl;
            in.close();
            return;
        }
        transposition_table[key] = value;
    }
    in.close();
}

std::pair<int, std::pair<int, int> > search(const std::string &key) {
    auto it = transposition_table.find(key);
    if (it != transposition_table.end()) {
        return it->second;
    } else {
        return std::make_pair(0, std::make_pair(0, 0));
    }
}

std::unordered_map<std::string, std::pair<int, std::pair<int, int> > > *getTable() {
    auto *p = &transposition_table;
    return p;
}