#include <SDL2/SDL.h>
#include "GameState.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Shooter Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // std::cout << "main: SDL_CreateRenderer finished." << std::endl;

    GameState gameState(renderer);

    bool running = true;
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();

    while (running) {
        // std::cout << "main: Start of game loop." << std::endl;
        Uint64 now = SDL_GetPerformanceCounter();
        float deltaTime = (float)(now - last) / freq;
        last = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            gameState.handleInput(event);
        }

        gameState.update(deltaTime);

        SDL_SetRenderDrawColor(renderer, 5, 5, 8, 255);
        SDL_RenderClear(renderer);

        gameState.render();
        
        SDL_RenderPresent(renderer);
        // std::cout << "main: End of game loop, after render." << std::endl;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}