// Player.h
#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <unordered_map>
#include "Bullet.h"
#include "Upgrade.h"
#include "ObjectPool.h" // Include ObjectPool for the shoot method
#include <SDL2/SDL.h> // Include SDL for SDL_Renderer

class GameState; // Forward declaration

enum class UpgradeTag {
    DAMAGE,
    FIRERATE,
    SPREAD,
    PIERCE,
    CRIT,
    STATUS
};

class Player {
public:
    float x, y;
    int hp;
    int fireRate;
    std::vector<Upgrade> activeUpgrades;
    float currentDX; // Stores player's intended movement direction
    float shootCooldown; // New: Manages time until next shot
    int baseDamage;
    int shotsFired;

    // Synergy Flags
    bool hasOverheat;
    bool hasShatter;
    bool hasExecute;

    std::unordered_map<UpgradeTag, int> upgrades;

    Player();
    void move(float dx);
    void shoot(ObjectPool<Bullet>& bulletPool, GameState& gs);
    void update(float deltaTime); // Added update method
    void applyUpgrade(const Upgrade& upgrade);
    void takeDamage(int damage);
    void render(SDL_Renderer* renderer); // Added render method

    void addUpgrade(UpgradeTag tag, GameState& gs);
    void checkSynergies(GameState& gs);

    // Old upgrade methods, now wrappers for the new system
    void upgradeDamage(GameState& gs);
    void upgradeFireRate(GameState& gs);
    void upgradeSpread(GameState& gs);
    // New upgrades needed for synergies
    void upgradePierce(GameState& gs);
    void upgradeCrit(GameState& gs);
    
    int totalUpgrades();
    bool hasAnySynergy();

private:
    void enableOverheat(GameState& gs);
    void enableShatter(GameState& gs);
    void enableExecute(GameState& gs);
};
#endif
