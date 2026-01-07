// Player.cpp
#include "Player.h"
#include "ObjectPool.h"
#include "GameState.h" // Now needed for the GameState& parameters
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Player::Player() : x(400.0f), y(550.0f), hp(100), fireRate(10), currentDX(0.0f), shootCooldown(0.0f), baseDamage(10), shotsFired(0), hasOverheat(false), hasShatter(false), hasExecute(false),
    upgrades{
        {UpgradeTag::DAMAGE, 0},
        {UpgradeTag::FIRERATE, 0},
        {UpgradeTag::SPREAD, 0},
        {UpgradeTag::PIERCE, 0},
        {UpgradeTag::CRIT, 0},
        {UpgradeTag::STATUS, 0}
    }
{
    // Body is now empty
}

void Player::move(float dx) {
    currentDX = dx;
}

void Player::shoot(ObjectPool<Bullet>& bulletPool, GameState& gs) {
    if (shootCooldown > 0.0f) return;

    shootCooldown = 1.0f / fireRate;
    shotsFired++;

    // Overheat synergy check
    if (hasOverheat && shotsFired % 8 == 0) {
        gs.spawnOverheatBlast();
    }

    int spreadAmount = upgrades[UpgradeTag::SPREAD] > 0 ? upgrades[UpgradeTag::SPREAD] + 1 : 1;
    
    float bulletSpeed = 900.0f;
    float baseAngle = -M_PI / 2.0f;

    if (spreadAmount > 1) {
        float angleStep = 0.15f;
        int center = spreadAmount / 2;
        for (int i = 0; i < spreadAmount; ++i) {
            float angle = baseAngle + (i - center) * angleStep;
            Bullet* b = bulletPool.acquire();
            if (b) {
                b->x = x;
                b->y = y;
                b->vx = cosf(angle) * bulletSpeed;
                b->vy = sinf(angle) * bulletSpeed;
                b->damage = baseDamage;
                b->isPlayerOwned = true;
                b->active = true;
                b->toDestroy = false;
                b->pierce = upgrades[UpgradeTag::PIERCE];
            }
        }
    } else {
        Bullet* b = bulletPool.acquire();
        if (b) {
            b->x = x;
            b->y = y;
            b->vx = cosf(baseAngle) * bulletSpeed;
            b->vy = sinf(baseAngle) * bulletSpeed;
            b->damage = baseDamage;
            b->isPlayerOwned = true;
            b->active = true;
            b->toDestroy = false;
            b->pierce = upgrades[UpgradeTag::PIERCE];
        }
    }
}

void Player::update(float deltaTime) {
    x += currentDX * deltaTime;
    if (x < 0) x = 0;
    if (x > GameState::SCREEN_WIDTH) x = GameState::SCREEN_WIDTH;

    if (shootCooldown > 0) {
        shootCooldown -= deltaTime;
    }
}

void Player::applyUpgrade(const Upgrade& upgrade) {
    activeUpgrades.push_back(upgrade);
    if (upgrade.type == "fireRate") {
        fireRate += upgrade.value;
    }
}

void Player::takeDamage(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Player::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect playerRect = {static_cast<int>(x - 10), static_cast<int>(y - 10), 20, 20};
    SDL_RenderFillRect(renderer, &playerRect);
}

void Player::addUpgrade(UpgradeTag tag, GameState& gs) {
    upgrades[tag]++;
    checkSynergies(gs);
}

void Player::checkSynergies(GameState& gs) {
    if (!hasOverheat && upgrades[UpgradeTag::DAMAGE] >= 3 && upgrades[UpgradeTag::FIRERATE] >= 2) {
        enableOverheat(gs);
    }
    if (!hasShatter && upgrades[UpgradeTag::SPREAD] >= 2 && upgrades[UpgradeTag::PIERCE] >= 1) {
        enableShatter(gs);
    }
    if (!hasExecute && upgrades[UpgradeTag::CRIT] >= 2 && upgrades[UpgradeTag::DAMAGE] >= 2) {
        enableExecute(gs);
    }
}

void Player::upgradeDamage(GameState& gs) { addUpgrade(UpgradeTag::DAMAGE, gs); baseDamage += 2; }
void Player::upgradeFireRate(GameState& gs) { addUpgrade(UpgradeTag::FIRERATE, gs); fireRate += 2; }
void Player::upgradeSpread(GameState& gs) { addUpgrade(UpgradeTag::SPREAD, gs); }
void Player::upgradePierce(GameState& gs) { addUpgrade(UpgradeTag::PIERCE, gs); }
void Player::upgradeCrit(GameState& gs) { addUpgrade(UpgradeTag::CRIT, gs); }

int Player::totalUpgrades() {
    int total = 0;
    for (auto const& pair : upgrades) {
        total += pair.second;
    }
    return total;
}

bool Player::hasAnySynergy() {
    return hasOverheat || hasShatter || hasExecute;
}

void Player::enableOverheat(GameState& gs) { 
    hasOverheat = true; 
    gs.triggerSynergyFeedback("OVERHEAT");
}
void Player::enableShatter(GameState& gs) { 
    hasShatter = true; 
    gs.triggerSynergyFeedback("SHATTER");
}
void Player::enableExecute(GameState& gs) { 
    hasExecute = true; 
    gs.triggerSynergyFeedback("EXECUTE");
}