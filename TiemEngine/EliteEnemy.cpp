#include "EliteEnemy.h"

// Pass EnemyType::A so the base constructor can initialise text objects and
// memory — derived constructors replace the sprite and stats immediately after.
EliteEnemy::EliteEnemy(EliteType eliteType)
    : Enemy(Enemy::EnemyType::A), eliteType(eliteType)
{
}
