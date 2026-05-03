#pragma once
#include <string>
#include "Enemy.h"

class LevelManager
{
private:
    int currentLevel = 1;
    static constexpr int MAX_LEVELS = 3;

public:
    int  GetLevel()   const { return currentLevel; }
    bool CanAdvance() const { return currentLevel < MAX_LEVELS; }
    void Advance()          { if (CanAdvance()) currentLevel++; }
    void Reset()            { currentLevel = 1; }

    std::string GetLevelText() const;

    void GetEnemyTypes(Enemy::EnemyType& a, Enemy::EnemyType& b, Enemy::EnemyType& c) const;

    // Level 1: 30-33 coins, Level 2: 33-36, Level 3: 36-39
    int RollCoins() const;
};
