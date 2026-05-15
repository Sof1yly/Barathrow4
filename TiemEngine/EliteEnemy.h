#pragma once
#include "Enemy.h"

// Shared base for all elite enemy types.
// Elites load their own stats/sprite in their own constructor
// after calling the Enemy base with a placeholder type.
class EliteEnemy : public Enemy
{
public:
    enum class EliteType { Row, Pattern };

    EliteEnemy(EliteType eliteType);
    virtual ~EliteEnemy() = default;

    EliteType getEliteType() const { return eliteType; }

protected:
    EliteType eliteType;
};
