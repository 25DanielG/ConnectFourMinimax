#ifndef BOARDGUI_HPP_
#define BOARDGUI_HPP_
#include "board.hpp"
void output(); // Main output function for all the connect four graphics
void update();
void input();
void draw();
void sdl_ellipse(int x0, int y0, int radiusX, int radiusY);
void updateBoard(Board &board);
#endif