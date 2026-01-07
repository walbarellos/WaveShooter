// GameState.cpp (melhorado)
#include "GameState.h"
#include <algorithm> // For std::sort, std::clamp
#include <cstdlib>   // For rand()
#include <time.h>    // For srand()
#include "Background.h" // Include Background.h

// Initialize static members
float GameState::screenShake = 0.0f;
int GameState::hitStopFrames = 0;
bool GameState::waveInProgress = false; // Initialize waveInProgress
std::vector<GameState::PendingSpawn> GameState::spawnQueue; // Initialize spawnQueue
float GameState::spawnTimer = 0.0f; // Initialize spawnTimer
int GameState::spawnIndex = 0; // Initialize spawnIndex
std::vector<GameState::DamageNumber> GameState::damageNumbers; // Initialize damageNumbers
float GameState::impactShake = 0.0f; // Initialize impactShake



GameState::GameState(SDL_Renderer* prenderer)
    : player(), // Default constructor for Player
      availableUpgrades(), // Default constructor for availableUpgrades
      currentWave(1),
      score(0),
      enemiesKilled(0),
      nextEliteAt(12),
      isGameOver(false),
      renderer(prenderer),
      bulletPool(100), // Initialize bullet pool with a size
      enemyPool(50),    // Initialize enemy pool with a size
      background(1, 1) // temporary
{
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    background = Background(w, h);

    // Initialize random seed
    srand(time(NULL));

    // Populate availableUpgrades with some initial upgrades
    availableUpgrades.push_back(Upgrade(1, "Spread Shot", "spread", 2, 1));
    availableUpgrades.push_back(Upgrade(2, "Fire Rate", "fireRate", 1, 1));

    waveInProgress = false; // Initialize waveInProgress
    spawnIndex = 0; // Initialize spawnIndex
    // Initially advance wave to spawn first enemies
    advanceWave();
}

const float PLAYER_SPEED = 300.0f; // pixels/segundo
const float BULLET_BASE_SPEED = 500.0f; // pixels/segundo


void GameState::handleInput(const SDL_Event& event) {
    // Usar deltaTime para suavidade (apenas teclas pressionadas)
    const Uint8* keystates = SDL_GetKeyboardState(nullptr);
    float dx = 0.0f;
    if (keystates[SDL_SCANCODE_LEFT]) dx -= PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_RIGHT]) dx += PLAYER_SPEED;

    player.move(dx); // Pass dx directly

    if (keystates[SDL_SCANCODE_SPACE]) { // Check if space is held down
        player.shoot(bulletPool, *this); // Use bulletPool here
    }
}

// checkCollision function implementation
bool GameState::checkCollision(Bullet* b, Enemy* e) {
    if (!b || !e) return false;
    float dx = b->x - e->x;
    float dy = b->y - e->y;

    // Use enemy's radius for collision detection
    return
        dx > -e->radius &&
        dx <  e->radius &&
        dy > -e->radius &&
        dy <  e->radius;
}

void GameState::update(float deltaTime) {
    if (isGameOver) return;

    float intensity = currentWave * 0.12f; // ajuste fino
    background.update(deltaTime, intensity); // Update background

    // Clamp dt pra evitar explosões em lag
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    // Hit stop logic
    if (hitStopFrames > 0) {
        hitStopFrames--;
        // Still allow some updates during hitstop for effects that shouldn't freeze
        // For example, screen shake decay, but not game logic
        screenShake *= 0.85f;
        // flash *= 0.9f; // Removed
        return; // Skip game logic updates during hitstop
    }

    // --- Player ---
    player.update(deltaTime); // Update player logic (e.g., cooldowns, invincibility frames)

    // --- Spawn Management ---
    spawnTimer += deltaTime;
    while (!spawnQueue.empty() && spawnTimer >= spawnQueue.front().delay) {
        PendingSpawn ps = spawnQueue.front();
        spawnQueue.erase(spawnQueue.begin());

        Enemy* e = enemyPool.acquire();
        if (e) {
            e->x = SCREEN_WIDTH * 0.5f + ps.xOffset; // Centered lane + offset
            e->y = -spawnIndex * 90.0f; // One after another off-screen
            e->hp = 30 + currentWave * 5; // Base HP + wave scaling
            e->maxHp = e->hp;
            e->type = ps.type;
            e->speed = 80.0f + currentWave * 5.0f; // Adjusted speed based on wave and type
            e->pattern = 0; // Default pattern for now
            e->active = true;
            e->hitTimer = 0.0f; // Reset hitTimer
            spawnIndex++; // Increment for next enemy in queue
        }
    }
    
    // --- Enemies ---
    auto& enemies = enemyPool.activeObjects;
    size_t i_enemy = 0;
    while (i_enemy < enemies.size()) {
        Enemy* e = enemies[i_enemy];
        e->update(deltaTime);

        // Check if enemy is off-screen or takes damage from player (not collision yet)
        if (e->hp <= 0 || e->y > SCREEN_HEIGHT + e->radius) { // Enemy off-screen or dead
            if (e->hp <= 0) { // It was actually killed
                enemiesKilled++;
                score += (e->type == 0) ? 10 : ((e->type == 1) ? 20 : 50); // Score based on enemy type
                if (e->isElite) {
                    onEliteKilled();
                }
            }

            // impacto visual
            screenShake = std::max(screenShake, 3.0f); // Intensify shake on enemy death
            
            enemyPool.releaseAt(i_enemy); // Release at current index. Element at i_enemy is replaced by last one.

            // Check for elite spawn AFTER releasing the enemy
            if (enemiesKilled >= nextEliteAt) {
                spawnElite();
                nextEliteAt += 10 + rand() % 6; // 10–15
            }
            // Do not increment i_enemy, as the new element at i_enemy needs to be processed.
        } else {
            ++i_enemy; // Only increment if current enemy was not released.
        }
    }

    // --- Bullets ---
    auto& bullets = bulletPool.activeObjects;
    size_t i_bullet = 0;
    while (i_bullet < bullets.size()) {
        Bullet* b = bullets[i_bullet];
        b->update(deltaTime);

        // Remove bullets off-screen or if marked for destruction
        if (b->toDestroy || b->y < 0 - 10 || b->y > SCREEN_HEIGHT + 10 || b->x < 0 - 10 || b->x > SCREEN_WIDTH + 10) {
            bulletPool.releaseAt(i_bullet); // Release at current index.
            // Do not increment i_bullet.
        } else {
            ++i_bullet; // Only increment if current bullet was not released.
        }
    }

    // --- Colisão (BULLET -> ENEMY) ---
    // Using the Y-sweep broad phase approach
    // First, sort enemies by Y for efficient sweep
    std::sort(enemies.begin(), enemies.end(),
        [](Enemy* a, Enemy* b) {
            return a->y < b->y;
        }
    );

    const float MAX_DIST_Y = 30.0f; // Adjustable
    for (Bullet* b : bullets) {
        if (!b->active || !b->isPlayerOwned) continue;

        float by = b->y;

        for (Enemy* e : enemies) {
            if (!e->active) continue;

            float dy_diff = e->y - by;

            if (dy_diff < -MAX_DIST_Y) continue; // enemy too far above
            if (dy_diff >  MAX_DIST_Y) break;    // Passed the window (because sorted!)

            // Now, check actual collision
            if (checkCollision(b, e)) {
                bool enemyKilled = false;
                if (player.hasExecute && e->hp < e->maxHp * 0.2f) {
                    e->hp = 0;
                    triggerExecuteFX();
                    enemyKilled = true;
                } else {
                    DamageEvent ev = e->takeDamage(b->damage);
                    // Create DamageNumber
                    DamageNumber dn;
                    dn.x = e->x;
                    dn.y = e->y - e->radius;
                    dn.value = ev.amount;
                    dn.critical = ev.critical;
                    dn.life = 0.8f; // Damage number life in seconds
                    damageNumbers.push_back(dn);

                    if (ev.critical) {
                        impactShake = std::min(impactShake + 2.0f, 4.0f); // Trigger impact shake on critical hit
                    }
                    if (e->hp <= 0) {
                        enemyKilled = true;
                    }
                }

                if (enemyKilled && player.hasShatter) {
                    spawnShatterFragments(e->x, e->y);
                }

                screenShake = std::max(screenShake, 1.5f); // Micro shake on bullet hit
                hitStopFrames = 3; // Apply hit stop

                if (b->pierce <= 0) {
                    b->toDestroy = true;
                    break; // Bullet is destroyed, so it can't hit another enemy
                } else {
                    b->pierce--;
                    // Don't break, allow bullet to continue
                }
            }
        }
    }

    // --- Player - Enemy collision ---
    // TODO: Implement player-enemy collision and damage handling

    // Decay visual effects
    screenShake *= 0.9f; // Decay screenShake
    impactShake *= 0.85f; // Decay impactShake
    // Removed flashTimer decay

    // --- Damage Numbers Update ---
    for (auto it = damageNumbers.begin(); it != damageNumbers.end(); ) {
        it->y -= deltaTime * 40.0f; // Float upwards
        it->life -= deltaTime;      // Decay life
        if (it->life <= 0) {
            it = damageNumbers.erase(it); // Remove if life is over
        } else {
            ++it;
        }
    }

    // --- Wave Management ---
    if (waveInProgress) {
        // A wave is considered complete if all enemies that were supposed to spawn have spawned
        // AND all currently active enemies are inactive.
        if (spawnQueue.empty() && enemyPool.activeObjects.empty()) {
            waveInProgress = false; // Current wave finished
        }
    }

    if (!waveInProgress) {
        advanceWave(); // Start next wave
    }

    // Game over
    if (player.hp <= 0) isGameOver = true;
}

        

        void GameState::render() { // Start of GameState::render definition

            // --- Limpa tela ---

            SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);

            SDL_RenderClear(renderer);

        

            // SCREEN SHAKE OFFSET

                        int shakeX = 0;

                        int shakeY = 0;

                        float totalShake = screenShake + impactShake; // Combine screenShake and impactShake

                        if (totalShake > 0.1f) {

                            int shakeAmount = (int)(totalShake * 2);

                            if (shakeAmount < 1) shakeAmount = 1; // Ensure shakeAmount is at least 1

                            shakeX = (rand() % shakeAmount) - (shakeAmount / 2); // Center around zero

                            shakeY = (rand() % shakeAmount) - (shakeAmount / 2); // Center around zero

                        }

        

            SDL_Rect vp{ shakeX, shakeY, SCREEN_WIDTH, SCREEN_HEIGHT };

                        SDL_RenderSetViewport(renderer, &vp);

            

                                    // --- Render background ---

            

                                    background.render(renderer);

                    

                        // --- Render player ---

                        player.render(renderer);

        

            // --- Render enemies ---

            for (Enemy* e : enemyPool.activeObjects) {

                if (e->active)

                    e->render(renderer);

            }

        

            // --- Render bullets ---

            for (Bullet* b : bulletPool.activeObjects) {

                if (b->active)

                    b->render(renderer);

            }

        

                                    // --- Render Damage Numbers ---

        

                                    // Helper to render a single digit using rectangles

        

                                    auto renderDigit = [&](int digit, int x_pos, int y_pos, float digit_scale, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {

        

                                        // Segment definitions for a 7-segment display (horizontal segments first, then vertical)

        

                                        //   ---0---

        

                                        //  |       |

        

                                        //  5       1

        

                                        //  |       |

        

                                        //   ---6---

        

                                        //  |       |

        

                                        //  4       2

        

                                        //  |       |

        

                                        //   ---3---

        

                                        bool segments[7];

        

                                        switch (digit) {

        

                                            case 0: for(int k=0; k<6; ++k) segments[k]=true; segments[6]=false; break; // 0

        

                                            case 1: segments[0]=false; segments[1]=true; segments[2]=true; segments[3]=false; segments[4]=false; segments[5]=false; segments[6]=false; break; // 1

        

                                            case 2: segments[0]=true; segments[1]=true; segments[2]=false; segments[3]=true; segments[4]=true; segments[5]=false; segments[6]=true; break; // 2

        

                                            case 3: segments[0]=true; segments[1]=true; segments[2]=true; segments[3]=true; segments[4]=false; segments[5]=false; segments[6]=true; break; // 3

        

                                            case 4: segments[0]=false; segments[1]=true; segments[2]=true; segments[3]=false; segments[4]=false; segments[5]=true; segments[6]=true; break; // 4

        

                                            case 5: segments[0]=true; segments[1]=false; segments[2]=true; segments[3]=true; segments[4]=false; segments[5]=true; segments[6]=true; break; // 5

        

                                            case 6: segments[0]=true; segments[1]=false; segments[2]=true; segments[3]=true; segments[4]=true; segments[5]=true; segments[6]=true; break; // 6

        

                                            case 7: segments[0]=true; segments[1]=true; segments[2]=true; segments[3]=false; segments[4]=false; segments[5]=false; segments[6]=false; break; // 7 (top, top-right, bottom-right)

        

                                            case 8: for(int k=0; k<7; ++k) segments[k]=true; break; // 8

        

                                            case 9: segments[0]=true; segments[1]=true; segments[2]=true; segments[3]=true; segments[4]=false; segments[5]=true; segments[6]=true; break; // 9

        

                                            default: for(int k=0; k<7; ++k) segments[k]=false; break; // Empty or error

        

                                        }

        

                        

        

                                                                                SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

        

                        

        

                                        

        

                        

        

                                                                                float seg_w = 2.0f * digit_scale; // Thickness of segments

        

                        

        

                                                                                float seg_h = 2.0f * digit_scale;

        

                        

        

                                                                                float seg_len = 6.0f * digit_scale; // Length of horizontal segments

        

                        

        

                                                                                float vert_len = 6.0f * digit_scale; // Length of vertical segments

        

                        

        

                                        

        

                        

        

                                                                                // Segment 0 (top)

        

                        

        

                                                                                if (segments[0]) { SDL_Rect rect = {(int)(x_pos), (int)(y_pos), (int)seg_len, (int)seg_h}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 1 (top-right)

        

                        

        

                                                                                if (segments[1]) { SDL_Rect rect = {(int)(x_pos + seg_len - seg_w), (int)(y_pos), (int)seg_w, (int)vert_len}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 2 (bottom-right)

        

                        

        

                                                                                if (segments[2]) { SDL_Rect rect = {(int)(x_pos + seg_len - seg_w), (int)(y_pos + vert_len + seg_h), (int)seg_w, (int)vert_len}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 3 (bottom)

        

                        

        

                                                                                if (segments[3]) { SDL_Rect rect = {(int)(x_pos), (int)(y_pos + 2 * vert_len + seg_h), (int)seg_len, (int)seg_h}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 4 (bottom-left)

        

                        

        

                                                                                if (segments[4]) { SDL_Rect rect = {(int)(x_pos), (int)(y_pos + vert_len + seg_h), (int)seg_w, (int)vert_len}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 5 (top-left)

        

                        

        

                                                                                if (segments[5]) { SDL_Rect rect = {(int)(x_pos), (int)(y_pos), (int)seg_w, (int)vert_len}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                                // Segment 6 (middle)

        

                        

        

                                                                                if (segments[6]) { SDL_Rect rect = {(int)(x_pos), (int)(y_pos + vert_len), (int)seg_len, (int)seg_h}; SDL_RenderFillRect(renderer, &rect); }

        

                        

        

                                                                            };

        

                        

        

                        

        

                                    for (const auto& dn : damageNumbers) {

        

                                        float scale = dn.critical ? 1.6f : 1.0f;

        

                                        float current_scale = scale * (dn.life / 0.8f + 0.2f); // Scale down as it fades

        

                        

        

                                        Uint8 cr_base = dn.critical ? 255 : 255;

        

                                        Uint8 cg_base = dn.critical ? 60  : 220;

        

                                        Uint8 cb_base = dn.critical ? 60  : 120;

        

                        

        

                                        Uint8 alpha = (Uint8)(dn.life / 0.8f * 255);

        

                                        if (alpha < 0) alpha = 0; // Ensure alpha is not negative

        

                        

        

                                        Uint8 cr = cr_base;

        

                                        Uint8 cg = cg_base;

        

                                        Uint8 cb = cb_base;

        

                                        // SDL_SetRenderDrawColor(renderer, cr, cg, cb, alpha); // Set per digit

        

                        

        

                                        int val = dn.value;

        

                                        if (val == 0) { // Handle zero case

        

                                            renderDigit(0, (int)dn.x, (int)dn.y, current_scale, cr, cg, cb, alpha);

        

                                        } else {

        

                                            std::string s_val = std::to_string(val); // Convert int to string

        

                                            int digit_x_offset = 0;

        

                                            for (char c : s_val) {

        

                                                int digit = c - '0';

        

                                                renderDigit(digit, (int)dn.x + digit_x_offset, (int)dn.y, current_scale, cr, cg, cb, alpha);

        

                                                digit_x_offset += (int)(8 * current_scale); // Advance X for next digit

        

                                            }

        

                                                                                }

        

                                        

        

                                                                            }

        

                                        } // This closes the GameState::render() function

        

                                        

        

                                        

        

                                        void GameState::advanceWave() {

        

                                            waveInProgress = true; // Set waveInProgress to true when a new wave starts

        

                                            currentWave++;

        

                                        

        

                                            spawnQueue.clear();

        

                                            spawnTimer = 0.0f;

        

                                            spawnIndex = 0; // Reset spawnIndex for new wave

        

                                        

        

                                            int total = 5 + currentWave * 2; // Total enemies in this wave

        

                                        

        

                                            for (int i = 0; i < total; ++i) {

        

                                                PendingSpawn ps;

        

                                                ps.delay = i * 0.5f;          // Rhythm of the wave (0.5s between spawns)

        

                                                ps.type  = rand() % 3;        // Assign a random type (0, 1, or 2)

        

                                                ps.xOffset = (rand() % 60) - 30; // Slight random horizontal offset

        

                                                spawnQueue.push_back(ps);

        

                                            }

        

                                        

        

                                            // Unlock upgrade example (simplified)

        

                                            if (currentWave % 3 == 0 && !availableUpgrades.empty()) {

        

                                                player.applyUpgrade(availableUpgrades[0]); // Apply first available upgrade

        

                                            }

        

                                        }
void GameState::spawnElite() {
    Enemy* e = enemyPool.acquire();
    if (!e) return;

    e->type = 99; // ELITE
    e->hp = 120
      + currentWave * 15
      + player.totalUpgrades() * 12;
    e->maxHp = e->hp;
    e->speed = 60;
    e->radius = 20;
    e->isElite = true;
    e->active = true;
    e->hitTimer = 0.0f;

    e->x = SCREEN_WIDTH * 0.5f;
    e->y = -80;

    // impacto visual
    screenShake = 8.0f;
    hitStopFrames = 6;
}

void GameState::onEliteKilled() {
    int roll = rand() % 5;

    if (roll == 0) player.upgradeDamage(*this);
    if (roll == 1) player.upgradeFireRate(*this);
    if (roll == 2) player.upgradeSpread(*this);
    if (roll == 3) player.upgradePierce(*this);
    if (roll == 4) player.upgradeCrit(*this);

    // feedback
    screenShake = 6.0f;
    hitStopFrames = 4;
    impactShake = 10.0f;
}

// Synergy methods
void GameState::spawnOverheatBlast() {
    // TODO: Implement Overheat
}

void GameState::spawnShatterFragments(float x, float y) {
    // TODO: Implement Shatter
}

void GameState::triggerExecuteFX() {
    // TODO: Implement Execute FX
}

void GameState::triggerSynergyFeedback(const std::string& name) {
    // TODO: Implement Synergy Feedback
    screenShake = 12.0f;
    hitStopFrames = 8;
    // spawnText(name, GOLD);
}