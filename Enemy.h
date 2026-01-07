// Enemy.h
#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h> // Include SDL for SDL_Renderer
#include "DamageEvent.h" // Include DamageEvent

class Enemy {
public:
    float x, y;
    int hp;
    int maxHp;
    int type; // Changed from std::string
    float speed;
    int pattern; // Changed from std::string
    bool active; // Added active member
    float radius; // Added radius for collision

    float pulsePhase = 0.0f; // Added for visual pulsing
    float prevX = 0.0f;     // Added for motion blur/trail
    float prevY = 0.0f;     // Added for motion blur/trail
    float hitTimer = 0.0f;  // New: for local hit feedback
    bool isElite;

    Enemy(); // Default constructor
    Enemy(float px, float py, int php, int ptype, float pspeed, int ppattern);
    void update(float deltaTime);
    DamageEvent takeDamage(int baseDamage); // Changed signature
    void render(SDL_Renderer* renderer); // Added render method
};

#endif