#include <SDL.h>
#include "../hsrc/boardgui.hpp"
#include <iostream>
#define WIDTH 1280
#define HEIGHT 720
bool running, fullscreen;
SDL_Renderer* renderer;
SDL_Window* window;
int frameCount, timerFPS, lastFrame, fps;
void update() {

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
}
void draw() {
    SDL_SetRenderDrawColor(renderer, 40, 43, 200, 255);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = WIDTH;
    rect.h = HEIGHT;
    SDL_RenderFillRect(renderer, &rect);
    ++frameCount;
    int timerFPS = SDL_GetTicks() - lastFrame;
    if(timerFPS  < (1000 / 60)) {
        SDL_Delay((1000 / 60) - timerFPS);
    }
    SDL_RenderPresent(renderer);
}
void output() {
    std::cerr << "Inside output function" << std::endl;
    running = true;
    fullscreen = false;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Failed at SDL_Init" << std::endl;
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
            //lastTime = lastFrame;
            fps = frameCount;
            frameCount = 0;
        }
        std::cout << "FPS: " << fps << std::endl;
        update();
        input();
        draw();
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}