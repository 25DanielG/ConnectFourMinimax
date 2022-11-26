#include <SDL.h>
#include <SDL_ttf.h>
#include "../hsrc/boardgui.hpp"
#include "../hsrc/board.hpp"
#include "../hsrc/minmax.hpp"
#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#define WIDTH 1280
#define HEIGHT 720
#define gameBorder 100
#define gameW (gameBorder * 2)
#define gameH (gameBorder * 1.5)
bool running, fullscreen, winner = false;
SDL_Renderer* renderer;
SDL_Window* window;
int frameCount = 0, timerFPS, lastFrame, fps;
Board gameBoard;
void update() {
    if(fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    if(!fullscreen) {
        SDL_SetWindowFullscreen(window, 0);
    }
    if(winner) {
        
    }
}
void input() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            running = false;
        }
    }
    const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
    if(keyStates[SDL_SCANCODE_ESCAPE]) {
        running = false;
    }
    if(keyStates[SDL_SCANCODE_F]) {
        fullscreen = !fullscreen;
    }
}
void draw() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Background
    SDL_Rect backRect;
    backRect.x = 0;
    backRect.y = 0;
    backRect.w = WIDTH;
    backRect.h = HEIGHT;
    SDL_RenderFillRect(renderer, &backRect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Rectangle
    SDL_Rect frontRect;
    frontRect.x = gameBorder;
    frontRect.y = gameBorder;
    frontRect.w = WIDTH - gameW;
    frontRect.h = HEIGHT - gameH;
    SDL_RenderDrawRect(renderer, &frontRect);

    std::vector<SDL_Rect> cols; // Column lines
    for(int i = 0; i < 6; ++i) {
        SDL_Rect single;
        single.x = gameBorder + (((WIDTH - (gameBorder * 2)) / 7) * (i + 1));
        single.y = gameBorder;
        single.w = 1;
        single.h = HEIGHT - (gameBorder * 1.5);
        cols.push_back(single);
    }
    for(int i = 0; i < 6; ++i) SDL_RenderDrawRect(renderer, &cols[i]);

    std::vector<SDL_Rect> rows; // Horizontal lines
    for(int i = 0; i < 5; ++i) {
        SDL_Rect single;
        single.x = gameBorder;
        single.y = gameBorder + (((HEIGHT - (gameBorder * 1.5)) / 6) * (i + 1));
        single.w = WIDTH - (gameBorder * 2);
        single.h = 1;
        rows.push_back(single);
    }
    for(int i = 0; i < 5; ++i) SDL_RenderDrawRect(renderer, &rows[i]);

    std::vector<std::vector<char> > matrix = gameBoard.getMatrixBoard();
    int pushX = ((WIDTH - (gameBorder * 2)) / 7);
    int startX = gameBorder + ((WIDTH - (gameBorder * 2)) / 14);
    int pushY = ((HEIGHT - (gameBorder * 1.5)) / 6);
    int startY = gameBorder + ((HEIGHT - (gameBorder * 1.5)) / 12);
    for(int i = 0; i < gameBoard.rows; ++i) {
        for(int j = 0; j < gameBoard.columns; ++j) {
            if(matrix[i][j] == '#')
                continue;
            if(matrix[i][j] == 'X')
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            for(int r = 0; r <= 30; ++r) {
                sdl_ellipse(startX + (pushX * j), startY + (pushY * i), r, r);
            }
        }
    }

    ++frameCount;
    int timerFPS = SDL_GetTicks() - lastFrame;
    if(timerFPS  < (1000 / 60)) {
        SDL_Delay((1000 / 60) - timerFPS);
    }
    SDL_RenderPresent(renderer);
}

void sdl_ellipse(int x0, int y0, int radiusX, int radiusY) {
    float pi  = 3.14159265358979323846264338327950288419716939937510;
    float pih = pi / 2.0; //half of pi

    //drew  28 lines with   4x4  circle with precision of 150 0ms
    //drew 132 lines with  25x14 circle with precision of 150 0ms
    //drew 152 lines with 100x50 circle with precision of 150 3ms
    const int prec = 27; // precision value; value of 1 will draw a diamond, 27 makes pretty smooth circles.
    float theta = 0;     // angle that will be increased each loop

    //starting point
    int x  = (float)radiusX * cos(theta);//start point
    int y  = (float)radiusY * sin(theta);//start point
    int x1 = x;
    int y1 = y;

    //repeat until theta >= 90;
    float step = pih/(float)prec; // amount to add to theta each time (degrees)
    for(theta = step; theta <= pih; theta += step) { //step through only a 90 arc (1 quadrant)
        //get new point location
        x1 = (float)radiusX * cosf(theta) + 0.5; //new point (+.5 is a quick rounding method)
        y1 = (float)radiusY * sinf(theta) + 0.5; //new point (+.5 is a quick rounding method)

        //draw line from previous point to new point, ONLY if point incremented
        if( (x != x1) || (y != y1) ) { //only draw if coordinate changed
            SDL_RenderDrawLine(renderer, x0 + x, y0 - y, x0 + x1, y0 - y1 ); //quadrant TR
            SDL_RenderDrawLine(renderer, x0 - x, y0 - y, x0 - x1, y0 - y1 ); //quadrant TL
            SDL_RenderDrawLine(renderer, x0 - x, y0 + y, x0 - x1, y0 + y1 ); //quadrant BL
            SDL_RenderDrawLine(renderer, x0 + x, y0 + y, x0 + x1, y0 + y1 ); //quadrant BR
        }
        //save previous points
        x = x1;//save new previous point
        y = y1;//save new previous point
    }
    //arc did not finish because of rounding, so finish the arc
    if(x != 0) {
        x = 0;
        SDL_RenderDrawLine(renderer, x0 + x, y0 - y, x0 + x1, y0 - y1 );//quadrant TR
        SDL_RenderDrawLine(renderer, x0 - x, y0 - y, x0 - x1, y0 - y1 );//quadrant TL
        SDL_RenderDrawLine(renderer, x0 - x, y0 + y, x0 - x1, y0 + y1 );//quadrant BL
        SDL_RenderDrawLine(renderer, x0 + x, y0 + y, x0 + x1, y0 + y1 );//quadrant BR
    }
}
void output() {
    running = true;
    fullscreen = false;
    static int lastTime = 0;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Failed at SDL_Init" << std::endl;
    }
    if(TTF_Init() == -1) {
        std::cerr << "Failed at TTF_Init" << std::endl;
    }
    if(SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
        std::cerr << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
    }
    SDL_SetWindowTitle(window, "Connect Four");
    SDL_ShowCursor(1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    while(running) {
        lastFrame = SDL_GetTicks();
        if(lastFrame >= (lastFrame + 1000)) {
            lastTime = lastFrame;
            fps = frameCount;
            frameCount = 0;
        }
        update();
        input();
        draw();
        performMove(gameBoard);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
}
void updateBoard(Board &board) {
    gameBoard = board;
}