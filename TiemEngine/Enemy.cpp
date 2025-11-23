#include "Enemy.h"

Enemy::Enemy()
{
	health = maxHealth;

}
void Enemy::setHealth(int h)
{
	health = h;
}

void Enemy::getDamage(int damage)
{
	health -= damage;
}