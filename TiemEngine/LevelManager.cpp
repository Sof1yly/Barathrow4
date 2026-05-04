#include "LevelManager.h"
#include <cstdlib>

std::string LevelManager::GetLevelText() const
{
    return "Level " + std::to_string(currentLevel);
}

void LevelManager::GetEnemyTypes(Enemy::EnemyType& a, Enemy::EnemyType& b, Enemy::EnemyType& c) const
{
    a = Enemy::EnemyType::A;
    b = Enemy::EnemyType::B;
    c = Enemy::EnemyType::C;
}

int LevelManager::RollCoins() const
{
    int minC = 17 + currentLevel * 3;
    int maxC = 20 + currentLevel * 3;
    return minC + rand() % (maxC - minC + 1);
}
