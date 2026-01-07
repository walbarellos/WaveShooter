// Wave.cpp
#include "Wave.h"

Wave::Wave(int pid, int pwaveNumber) // Removed pspawnPattern
    : id(pid), waveNumber(pwaveNumber) {}

void Wave::spawnEnemies() {
    // Exemplo: spawn 5 inimigos tipo A
    for (int i = 0; i < 5; ++i) {
        enemies.push_back(Enemy(100 + i * 100, 50, 20, 0, 1.0f, 0)); // 'A' -> 0, 'down' -> 0
    }
}

bool Wave::isComplete() {
    return enemies.empty();
}