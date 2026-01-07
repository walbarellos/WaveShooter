// Enemy.cpp
#include "Enemy.h"
#include <SDL2/SDL.h> // Include SDL for rendering
#include <cmath>      // Include cmath for sinf
#include <algorithm>  // Required for std::min

// Removed #include <string>

Enemy::Enemy() : x(0), y(0), hp(0), maxHp(0), type(0), speed(0), pattern(0), active(false), radius(20.0f), hitTimer(0.0f), isElite(false) {}

Enemy::Enemy(float px, float py, int php, int ptype, float pspeed, int ppattern)
    : x(px), y(py), hp(php), maxHp(php), type(ptype), speed(pspeed), pattern(ppattern), active(false), radius(20.0f), hitTimer(0.0f), isElite(false) {}

void Enemy::update(float deltaTime) {
    // Store previous position before updating current position
    prevX = x;
    prevY = y;

    // Movement based on pattern
    if (pattern == 0) { // Straight down
        y += speed * deltaTime;
    } else if (pattern == 1) { // Wavy movement
        y += speed * deltaTime;
        x += sinf(y * 0.03f) * 40 * deltaTime; // Adjust 0.03f and 40 for desired wave
    } else if (pattern == 2) { // Slower, heavier movement (e.g., for 'C' type)
        y += speed * deltaTime * 0.8f;
    }

    // Advance pulse phase for visual effect
    pulsePhase += deltaTime * 6.0f; // velocidade do pulso

    // Update hitTimer
    if (hitTimer > 0.0f) hitTimer -= deltaTime;
    if (hitTimer < 0) hitTimer = 0;
}

DamageEvent Enemy::takeDamage(int baseDamage) {
    DamageEvent ev{};
    
    // chance de crítico
    float critChance = 0.12f;
    bool crit = ((float)rand() / RAND_MAX) < critChance;

    int dmg = baseDamage;
    if (crit) dmg = (int)(dmg * 1.8f);

    hp -= dmg;
    if (hp < 0) hp = 0;

    hitTimer = 0.12f;

    ev.amount = dmg;
    ev.critical = crit;
    return ev;
}

void Enemy::render(SDL_Renderer* renderer) {
    // --- Normaliza HP ---
    float hpRatio = maxHp > 0 ? (float)hp / maxHp : 0.0f;
    if (hpRatio < 0.0f) hpRatio = 0.0f;
    if (hpRatio > 1.0f) hpRatio = 1.0f;

    // --- Pulso orgânico ---
    float pulse = 0.5f + 0.5f * sinf(pulsePhase);
    float size = radius * (0.9f + pulse * 0.15f);

    // --- Cor baseada no tipo ---
    Uint8 r_base = 180, g_base = 60, b_base = 60;
    if (type == 1) { r_base = 60; g_base = 160; b_base = 255; } // Type 'B'
    if (type == 2) { r_base = 255; g_base = 120; b_base = 40; } // Type 'C'

    // --- Intensidade pelo HP ---
    Uint8 r_final = (Uint8)(r_base * (0.4f + 0.6f * hpRatio));
    Uint8 g_final = (Uint8)(g_base * (0.4f + 0.6f * hpRatio));
    Uint8 b_final = (Uint8)(b_base * (0.4f + 0.6f * hpRatio));

    // --- Mix with white for hitFlash ---
    float hitIntensity = hitTimer > 0 ? (hitTimer / 0.12f) : 0.0f;
    r_final = (Uint8)std::min(255.0f, r_final + hitIntensity * (255 - r_final));
    g_final = (Uint8)std::min(255.0f, g_final + hitIntensity * (255 - g_final));
    b_final = (Uint8)std::min(255.0f, b_final + hitIntensity * (255 - b_final));

    // --- Rastro temporal ---
    SDL_SetRenderDrawColor(renderer, r_final, g_final, b_final, 40);
    SDL_RenderDrawLine(
        renderer,
        (int)prevX, (int)prevY,
        (int)x, (int)y
    );

    // --- Glow externo (camadas baratas) ---
    for (int i = 3; i >= 1; --i) {
        Uint8 alpha = (Uint8)(30 * i);
        SDL_SetRenderDrawColor(renderer, r_final, g_final, b_final, alpha);

        SDL_Rect glow = {
            (int)(x - size - i * 2),
            (int)(y - size - i * 2),
            (int)((size * 2) + i * 4),
            (int)((size * 2) + i * 4)
        };

        SDL_RenderFillRect(renderer, &glow);
    }

    // --- Núcleo ---
    SDL_SetRenderDrawColor(renderer, r_final, g_final, b_final, 220);
    SDL_Rect core = {
        (int)(x - size),
        (int)(y - size),
        (int)(size * 2),
        (int)(size * 2)
    };
    SDL_RenderFillRect(renderer, &core);
}
