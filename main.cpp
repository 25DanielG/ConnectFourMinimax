#include <iostream>
#include <fstream>
#include <string>
#include "./src_hpp/boardgui.hpp"
#include "./src_hpp/hash.hpp"
#include "./src_hpp/minmax.hpp"
const std::string file_path = "transposition_table.txt";
const bool train = false;
const bool train_depth = 8;

int main() {
    std::ifstream in(file_path);
    if (!in) {
        std::cerr << "Error: File " << file_path << " does not exist." << std::endl;
        return 0;
    }
    load(getTable(), file_path);

    if(train) {
        trainComputer(train_depth);
    } else {
        output(); // Call graphics
    }
    return 0;
}