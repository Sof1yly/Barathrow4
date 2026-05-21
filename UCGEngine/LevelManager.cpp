#include "LevelManager.h"
#include <cstdlib>

std::string LevelManager::GetLevelText() const
{
    return "Level " + std::to_string(currentLevel);
}

bool LevelManager::IsShopOnlyLevel() const
{
    return currentLevel == 4  ||
           currentLevel == 9  ||
           currentLevel == 14 ||
           currentLevel == 19;
}

bool LevelManager::IsEliteLevel() const
{
    return currentLevel == 5  ||
           currentLevel == 10 ||
           currentLevel == 15;
}

LevelConfig LevelManager::BuildConfig(int level)
{
    using T = Enemy::EnemyType;

    const std::vector<T> abe = { T::A, T::B, T::E };
    const std::vector<T> cdf = { T::C, T::D, T::F };
    const std::vector<T> gh  = { T::G, T::H };

    LevelConfig cfg;

    switch (level)
    {
    // ---- Act 1: A-C enemies ------------------------------------------------
    case 1:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 2 } };
        break;
    case 2:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 3 } };
        break;
    case 3:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 4 } };
        break;
    case 4:
        cfg.type = LevelConfig::Type::Shop;
        break;
    case 5:
        cfg.type = LevelConfig::Type::Elite1;
        break;

    // ---- Act 2: D-F enemies ------------------------------------------------
    case 6:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { cdf, 2 } };
        break;
    case 7:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { cdf, 3 } };
        break;
    case 8:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { cdf, 4 } };
        break;
    case 9:
        cfg.type = LevelConfig::Type::Shop;
        break;
    case 10:
        cfg.type = LevelConfig::Type::Elite2;
        break;

    // ---- Act 3: G-H + one D-F enemy ----------------------------------------
    case 11:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { gh, 2 }, { cdf, 1 } };
        break;
    case 12:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { gh, 3 }, { cdf, 1 } };
        break;
    case 13:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { gh, 4 }, { cdf, 1 } };
        break;
    case 14:
        cfg.type = LevelConfig::Type::Shop;
        break;
    case 15:
        cfg.type = LevelConfig::Type::Elite3;
        break;

    // ---- Act 4: mixed pools ------------------------------------------------
    case 16:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 3 }, { cdf, 2 }, { gh, 1 } };
        break;
    case 17:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 2 }, { cdf, 2 }, { gh, 2 } };
        break;
    case 18:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 1 }, { cdf, 2 }, { gh, 3 } };
        break;
    case 19:
        cfg.type = LevelConfig::Type::Shop;
        break;
    case 20:
        cfg.type = LevelConfig::Type::Boss;
        break;

    default:
        cfg.type   = LevelConfig::Type::Combat;
        cfg.groups = { { abe, 3 } };
        break;
    }

    return cfg;
}

int LevelManager::RollCoins() const
{
    int minC = 17 + currentLevel * 3;
    int maxC = 20 + currentLevel * 3;
    return minC + rand() % (maxC - minC + 1);
}
