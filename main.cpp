#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "./src_hpp/boardgui.hpp"
#include "./src_hpp/hash.hpp"
#include "./src_hpp/minmax.hpp"
using namespace std::chrono;
const std::string file_path = "transposition_table.txt";
const bool train = false;
const bool train_depth = 8;

int main() {
    std::ifstream in(file_path);
    if (!in) {
        std::cerr << "Error: File " << file_path << " does not exist." << std::endl;
        return 0;
    }
    auto start = high_resolution_clock::now();

    load(getTable(), file_path);
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cerr << "Loaded transposition table in " << duration.count() << " milliseconds." << std::endl;

    if(train) {
        trainComputer(train_depth);
    } else {
        output(); // Call graphics
    }
    return 0;
}