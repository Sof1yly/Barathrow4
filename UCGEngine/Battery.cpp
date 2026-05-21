#include "Battery.h"
#include <iostream>
#include <algorithm>

Battery::Battery()
    : Enemy(EnemyType::D)
{
    maxHealth = 10;
    health    = maxHealth;
    damage    = 0;
    moveRange = 0;

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    // Replace the default sprite with the battery sprite sheet (3 rows x 7 cols)
    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Boss/battery.png", 3, 7);
    objSprite->SetSize(200.0f, -200.0f); // TODO: adjust size to match battery art

    PlayBatteryIdle();
}

void Battery::PlayBatteryIdle()
{
    if (objSprite)
        objSprite->SetAnimationLoop(0, 0, 1, 0);
}

void Battery::PlayIdleAnimation()
{
    PlayBatteryIdle();
}

void Battery::getDamage(int inDamage)
{
    // Do NOT call Enemy::getDamage — it sets isDead immediately and plays wrong anims.
    int total = inDamage + corruptionStacks;
    health -= total;
    if (health < 0) health = 0;

    if (corruptionStacks > 0)
        std::cout << "[Battery] Takes " << corruptionStacks
                  << " extra corruption damage (total " << total << ")\n";

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    if (!objSprite) return;

    if (health <= 0)
    {
        // Start death animation; isDead will be set once it finishes
        isDeadAnimating = true;
        objSprite->SetAnimationOnce(2, 0, 7, 120); // row 2, 7 frames, 120ms each
    }
    else
    {
        // Damage flash animation, then return to idle
        isTakingDamage = true;
        damageTimer    = 0.0f;
        damageDuration = 5 * 0.100f; // 5 frames @ 100ms
        objSprite->SetAnimationOnce(1, 0, 5, 100); // row 1, 5 frames, 100ms each
    }
}

void Battery::Update(float dt)
{
    if (isDeadAnimating)
    {
        // Wait for the death animation to complete, then mark truly dead
        if (objSprite && objSprite->IsFinished())
        {
            isDeadAnimating = false;
            isDead = true;
        }
        // Still call base update so the sprite itself ticks
        Enemy::Update(dt);
        return;
    }

    Enemy::Update(dt);

    // After a damage animation ends, Enemy::Update calls PlayIdleAnimation() which
    // plays the generic row-0 idle — that's exactly what we want for Battery too.
}
