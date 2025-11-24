#include "Enemy.h"

Enemy::Enemy()
{
	health = maxHealth;
    patterns = {
            AttackPattern::fromGrid({
                ".X.",
                "XXX",
                ".X."
            }, 'X'),

            AttackPattern::fromGrid({
                "XXX",
                "XXX",
                "XXX"
            }, 'X'),

            AttackPattern::fromGrid({
                "..X..",
                ".XXX.",
                "XXXXX",
                ".XXX.",
                "..X.."
            }, 'X'),

            AttackPattern::fromGrid({
                "...XX",
            }, 'X'),
    };

}
void Enemy::setHealth(int h)
{
	health = h;
}

void Enemy::getDamage(int damage)
{
	health -= damage;
}