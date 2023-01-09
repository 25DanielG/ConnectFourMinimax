#ifndef BOARDGUI_HPP_
#define BOARDGUI_HPP_
#include "board.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
void output(); // Main output function for all the connect four graphics
void update();
void input();
void mousePress(SDL_MouseButtonEvent& b);
void draw();
void sdl_ellipse(int x0, int y0, int radiusX, int radiusY);
void updateBoard(Board &board);
#endif