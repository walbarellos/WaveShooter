// Upgrade.cpp
#include "Upgrade.h"

Upgrade::Upgrade(int pid, std::string pname, std::string ptype, int pvalue, int punlockedWave)
    : id(pid), name(pname), type(ptype), value(pvalue), unlockedWave(punlockedWave) {}
