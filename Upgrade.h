// Upgrade.h
#ifndef UPGRADE_H
#define UPGRADE_H

#include <string>

struct Upgrade {
    int id;
    std::string name;
    std::string type;
    int value;
    int unlockedWave;

    Upgrade(int pid, std::string pname, std::string ptype, int pvalue, int punlockedWave);
};

#endif