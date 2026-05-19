#pragma once
#include <string>
#include <vector>
#include "Enemy.h"

struct EnemyGroup
{
    std::vector<Enemy::EnemyType> pool;
    int count = 0;
};

struct LevelConfig
{
    enum class Type { Combat, Shop, Elite1, Elite2, EliteRandom, Boss };

    Type                    type = Type::Combat;
    std::vector<EnemyGroup> groups; // populated for Combat levels only
};

class LevelManager
{
public:
    int  GetLevel()   const { return currentLevel; }
    bool CanAdvance() const { return currentLevel < MAX_LEVELS; }

    void Advance()           { if (CanAdvance()) currentLevel++; }
    void Reset()             { currentLevel = 1; }
    void SetLevel(int level) { currentLevel = level; }

    bool IsShopOnlyLevel() const;
    bool IsBossLevel()     const { return currentLevel == 20; }
    bool IsEliteLevel()    const;

    LevelConfig GetCurrentConfig() const { return BuildConfig(currentLevel); }

    std::string GetLevelText() const;
    int         RollCoins()    const;

private:
    int currentLevel = 1;
    static constexpr int MAX_LEVELS = 20;

    static LevelConfig BuildConfig(int level);
};
