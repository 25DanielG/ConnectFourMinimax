#include <SDL.h>
#include <SDL_ttf.h>
#include "../hsrc/boardgui.hpp"
#include <iostream>
#define WIDTH 1280
#define HEIGHT 720
bool running, fullscreen, winner = false;
SDL_Renderer* renderer;
SDL_Window* window;
int frameCount = 0, timerFPS, lastFrame, fps;
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
    SDL_SetRenderDrawColor(renderer, 40, 40, 200, 255);
    SDL_Rect backRect;
    backRect.x = 0;
    backRect.y = 0;
    backRect.w = WIDTH;
    backRect.h = HEIGHT;
    SDL_RenderFillRect(renderer, &backRect);
    int gameBorder = 200;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect frontRect;
    frontRect.x = gameBorder;
    frontRect.y = gameBorder;
    frontRect.w = WIDTH - (gameBorder * 2);
    frontRect.h = HEIGHT - (gameBorder * 2);
    SDL_RenderDrawRect(renderer, &frontRect);
    ++frameCount;
    int timerFPS = SDL_GetTicks() - lastFrame;
    if(timerFPS  < (1000 / 60)) {
        SDL_Delay((1000 / 60) - timerFPS);
    }
    SDL_RenderPresent(renderer);
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
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
}