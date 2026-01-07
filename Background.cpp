#include "Background.h"
#include <cmath>

Background::Background(int w, int h)
    : width(w), height(h), time(0.0f), pulse(0.0f) {}

void Background::update(float dt, float intensity) {
    time += dt;
    pulse += dt * (1.0f + intensity * 2.5f);
}

void Background::render(SDL_Renderer* r) {
    if (width <= 0 || height <= 0) return;

    // ---------- FUNDO BASE ----------
    SDL_SetRenderDrawColor(r, 4, 6, 12, 255);
    SDL_RenderClear(r);

    // ---------- NEBLINA (DEPTH) ----------
    for (int y = 0; y < height; y += 4) {
        float fog = (float)y / height;
        Uint8 alpha = (Uint8)(fog * 60);

        SDL_SetRenderDrawColor(r, 20, 30, 50, alpha);
        SDL_RenderDrawLine(r, 0, y, width, y);
    }

    // ---------- ONDAS DE FUNDO ----------
    SDL_SetRenderDrawColor(r, 40, 70, 120, 35);

    for (int x = 0; x < width; x += 4) {
        float wave =
            sinf(x * 0.018f + time * 1.2f) * 20 +
            sinf(x * 0.008f + time * 0.7f) * 15;

        SDL_RenderDrawLine(
            r,
            x,
            (int)(height * 0.25f + wave),
            x,
            height
        );
    }

    // ---------- CORREDOR REATIVO ----------
    SDL_SetRenderDrawColor(r, 90, 140, 220, 55);

    for (int y = 0; y < height; y += 6) {

        float distortion =
            sinf(y * 0.02f + pulse) * 60 +
            sinf(y * 0.006f + pulse * 0.5f) * 40;

        float center =
            width / 2 +
            distortion;

        SDL_RenderDrawLine(
            r,
            (int)(center - 200),
            y,
            (int)(center + 200),
            y
        );
    }

    // ---------- PULSOS DE WAVE ----------
    SDL_SetRenderDrawColor(r, 120, 180, 255, 45);

    for (int i = 0; i < 6; ++i) {
        float py = pulse * 80 + i * 120;
        while (py > height) py -= height;
        SDL_RenderDrawLine(r, 0, (int)py, width, (int)py);
    }

    // Vignette
    for (int i = 0; i < 120; i += 4) {
        Uint8 alpha = (Uint8)(i * 1.5f);
        SDL_SetRenderDrawColor(r, 0, 0, 0, alpha);

        SDL_Rect top    = {0, i, width, 4};
        SDL_Rect bottom = {0, height - i, width, 4};
        SDL_Rect left   = {i, 0, 4, height};
        SDL_Rect right  = {width - i, 0, 4, height};

        SDL_RenderFillRect(r, &top);
        SDL_RenderFillRect(r, &bottom);
        SDL_RenderFillRect(r, &left);
        SDL_RenderFillRect(r, &right);
    }
}