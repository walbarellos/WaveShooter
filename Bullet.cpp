// Bullet.cpp
#include "Bullet.h"
#include <SDL2/SDL.h> // Include SDL for rendering
#include <cmath>      // Include cmath for sinf

Bullet::Bullet()
    : x(0), y(0), vx(0), vy(0), damage(0), pierce(0), isPlayerOwned(false), active(false), toDestroy(false),
      speed(0), type(BULLET_LASER), baseAngle(0), wavePhase(0) {}

Bullet::Bullet(float px, float py, float pvx, float pvy, int pdamage, bool playerOwned)
    : x(px), y(py), vx(pvx), vy(pvy), damage(pdamage), pierce(0), isPlayerOwned(playerOwned), active(false), toDestroy(false),
      speed(0), type(BULLET_LASER), baseAngle(0), wavePhase(0) {
    
    float len = sqrtf(pvx*pvx + pvy*pvy);
    if (len < 0.0001f) { // Check for near-zero velocity
        active = false; // Deactivate if velocity is zero to prevent division by zero
        return;
    }
    speed = len;
    baseAngle = atan2f(pvy, pvx);
}

void Bullet::update(float deltaTime) {
    if (type == BULLET_PLASMA) {
        float wave = sinf(wavePhase) * 0.08f; // Adjusted for more subtle wave
        float angle = baseAngle + wave;
        vx = cosf(angle) * speed;
        vy = sinf(angle) * speed;
        wavePhase += deltaTime * 8.0f;
    }

    x += vx * deltaTime; // Use vx, vy for velocity
    y += vy * deltaTime;
}

void Bullet::render(SDL_Renderer* r) {
    switch (type) {
        case BULLET_LASER: {
            SDL_SetRenderDrawColor(r, 255, 220, 120, 255); // Trail color
            SDL_RenderDrawLine(r, (int)x, (int)y, (int)(x - vx * 0.015f), (int)(y - vy * 0.015f));
            
            SDL_Rect core = {(int)x - 2, (int)y - 10, 4, 12}; // Core as a rectangle
            SDL_RenderFillRect(r, &core);
            break;
        }
        case BULLET_SPREAD: {
            SDL_SetRenderDrawColor(r, 180, 255, 180, 200);
            SDL_RenderDrawLine(r, (int)x, (int)y, (int)(x - vx * 0.02f), (int)(y - vy * 0.02f));
            SDL_Rect core = {(int)x - 2, (int)y - 2, 4, 4};
            SDL_RenderFillRect(r, &core);
            break;
        }
        case BULLET_PLASMA: {
            SDL_SetRenderDrawColor(r, 180, 120, 255, 160);
            int plasmaSize = 3 + (int)(sinf(wavePhase * 2.0f) * 1.5f);
            SDL_Rect plasmaRect = {(int)x - plasmaSize, (int)y - plasmaSize, plasmaSize * 2, plasmaSize * 2};
            SDL_RenderFillRect(r, &plasmaRect);
            break;
        }
    }
}