// GameState.h
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Wave.h"
#include "Upgrade.h"
#include "ObjectPool.h" // Include ObjectPool
#include "Background.h"

class GameState {
public:
    Player player;
    // std::vector<Bullet> bullets; // Removed, now managed by ObjectPool
    std::vector<Upgrade> availableUpgrades;
    int currentWave;
    int score;
    int enemiesKilled;
    int nextEliteAt;
    bool isGameOver;
    SDL_Renderer* renderer;

    ObjectPool<Bullet> bulletPool; // Add bullet pool
    ObjectPool<Enemy> enemyPool;   // Add enemy pool
    Background background;

    static const int SCREEN_WIDTH = 800; // Define screen dimensions
    static const int SCREEN_HEIGHT = 600;

    GameState(SDL_Renderer* prenderer);
    void handleInput(const SDL_Event& event);
    void update(float deltaTime);
    void render();
    // void checkCollisions(); // Removed, will be integrated into update with juice
    void advanceWave();

    // Synergy methods
    void spawnOverheatBlast();
    void spawnShatterFragments(float x, float y);
    void triggerExecuteFX();
    void triggerSynergyFeedback(const std::string& name);

private:
    // Game juice variables
    static float screenShake;
    // static float flash; // Removed
    static int hitStopFrames;
    // static float pressure; // Removed
    // static float flashTimer; // Removed
    static bool waveInProgress; // New: Manages current wave state

    // Wave Pacing variables
    struct PendingSpawn {
        float delay;
        int type;
        float xOffset; // New: for slight horizontal variation
        // int pattern; // Can be added later if needed
    };
    static std::vector<PendingSpawn> spawnQueue;
    static float spawnTimer;
        static int spawnIndex; // New: to keep track of spawned enemies in a wave
    
        // Damage Numbers
        struct DamageNumber {
            float x, y;
            int value;
            float life;
            bool critical;
        };
        static std::vector<DamageNumber> damageNumbers;
    static float impactShake; // New: for screen impact effect
    
        // Collision helper
    bool checkCollision(Bullet* b, Enemy* e);
    void spawnElite();
    void onEliteKilled();
};

#endif