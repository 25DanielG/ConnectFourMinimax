#include <iostream>
#include "./src_hpp/boardgui.hpp"
#include "./src_hpp/hash.hpp"

int main() {
    output(); // Call graphics

    /* add_to_table("board1", 3, 5);
    add_to_table("board2", 4, 7);
    save("transposition_table.bin");

    load("transposition_table.bin");
    auto transposition_table = *getTable();
    for (auto &[key, value] : transposition_table) {
        std::cout << "key: " << key << " depth: " << value.first << " move: " << value.second << std::endl;
    } */

    return 0;
}