// Bullet.h
#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h> // Include SDL for SDL_Renderer
#include <cmath> // For sinf, cosf

// Bullet Types
enum BulletType {
    BULLET_LASER = 0,
    BULLET_SPREAD = 1,
    BULLET_PLASMA = 2
};
class Bullet {
public:
    float x, y, vx, vy; // Changed from dx, dy to vx, vy
    int damage;
    int pierce;
    bool isPlayerOwned;
    bool active; // Added active member
    bool toDestroy; // Added for deferred destruction

    float speed;    // New: magnitude of velocity
    int type;       // New: type of bullet for visual/behavioral distinction
    float baseAngle; // New: for oscillating/wavy patterns (e.g., plasma)
    float wavePhase; // New: for oscillating/wavy patterns (e.g., plasma)

    Bullet(); // Default constructor
    Bullet(float px, float py, float pvx, float pvy, int pdamage, bool playerOwned); // Updated signature
    void update(float deltaTime);
    void render(SDL_Renderer* r); // Added render method
};

#endif