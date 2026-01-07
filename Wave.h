// Wave.h
#ifndef WAVE_H
#define WAVE_H

#include <vector>
#include "Enemy.h"

class Wave {
public:
    int id;
    int waveNumber;
    std::vector<Enemy> enemies;
    // std::string spawnPattern; // Removed

    Wave(int pid, int pwaveNumber); // Updated constructor
    void spawnEnemies();
    bool isComplete();
};

#endif
