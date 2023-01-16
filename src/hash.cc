#include <iostream>
#include <fstream>
#include <unordered_map>

std::unordered_map<std::string, std::pair<int, int> > transposition_table;

void add_to_table(std::string board, int depth, int move) {
    transposition_table[board] = std::make_pair(depth, move);
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
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(size));
    for (uint64_t i = 0; i < size; ++i) {
        uint64_t key_size;
        in.read(reinterpret_cast<char *>(&key_size), sizeof(key_size));
        std::string key(key_size, ' ');
        in.read(const_cast<char*>(key.c_str()), key_size);
        std::pair<int, int> value;
        in.read(reinterpret_cast<char *>(&value), sizeof(value));
        transposition_table[key] = value;
    }
    in.close();
}

std::pair<int, int> search(const std::string &key) {
    auto it = transposition_table.find(key);
    if (it != transposition_table.end()) {
        return it->second;
    } else {
        return {0, 0};
    }
}

std::unordered_map<std::string, std::pair<int, int> > *getTable() {
    auto *p = &transposition_table;
    return p;
}