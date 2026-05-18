#include "EliteEnemy.h"
#include <algorithm>
#include <iostream>

EliteEnemy::EliteEnemy(EliteType eliteType)
    : Enemy(Enemy::EnemyType::A), eliteType(eliteType)
{
}

void EliteEnemy::UpdateFacing(glm::vec3 playerWorldPos)
{
    if (!objSprite) return;

    bool nowRight = (playerWorldPos.x > objSprite->GetPosition().x);
    if (nowRight == facingRight) return;

    facingRight = nowRight;
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);
}

void EliteEnemy::getDamage(int dmg)
{
    if (isDeadAnimating) return;

    int total = dmg + corruptionStacks;
    health -= total;

    if (health <= 0)
    {
        health = 0;
        isDeadAnimating = true;
        // isDead stays false until the death animation finishes (checked in Update)
        PlayDeathAnimation();
    }
    else
    {
        PlayDamageAnimation();
        isTakingDamage = true;
        damageTimer    = 0.0f;
    }

    if (corruptionStacks > 0)
        std::cout << "[Corrupt] Elite takes " << corruptionStacks << " extra damage (total " << total << ")\n";

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);
}
