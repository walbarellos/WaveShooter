#pragma once
#include <SDL2/SDL.h>

class Background {
public:
    Background(int w, int h);

    void update(float dt, float intensity); // intensity = wave pressure
    void render(SDL_Renderer* r);

private:
    int width, height;
    float time;
    float pulse;
};