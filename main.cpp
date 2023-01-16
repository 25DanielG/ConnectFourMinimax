#include <iostream>
#include <fstream>
#include "./src_hpp/boardgui.hpp"
#include <string>
#include "./src_hpp/hash.hpp"
const std::string file_path = "transposition_table.bin";

int main() {
    std::ifstream in(file_path);
    if (!in) {
        std::cerr << "Error: File " << file_path << " does not exist." << std::endl;
        return 0;
    }
    load(file_path);

    output(); // Call graphics
    return 0;
}