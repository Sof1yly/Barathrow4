#include "LevelManager.h"
#include <cstdlib>

std::string LevelManager::GetLevelText() const
{
    return "Level " + std::to_string(currentLevel);
}

void LevelManager::GetEnemyTypes(Enemy::EnemyType& a, Enemy::EnemyType& b, Enemy::EnemyType& c) const
{
    switch (currentLevel)
    {
    case 2:
        a = Enemy::EnemyType::D;
        b = Enemy::EnemyType::E;
        c = Enemy::EnemyType::F;
        break;
    case 3:
        a = Enemy::EnemyType::G;
        b = Enemy::EnemyType::H;
        c = Enemy::EnemyType::I;
        break;
    default:
        a = Enemy::EnemyType::A;
        b = Enemy::EnemyType::B;
        c = Enemy::EnemyType::C;
        break;
    }
}

int LevelManager::RollCoins() const
{
    int minC = 27 + currentLevel * 3;
    int maxC = 30 + currentLevel * 3;
    return minC + rand() % (maxC - minC + 1);
}
