#include "Boss.h"

Boss::Boss()
    : Enemy(EnemyType::C)
{
    maxHealth = 500;
    health = maxHealth;

    damage = 25;

    moveRange = 0;

    if (objSprite)
    {
        delete objSprite;
        objSprite = nullptr;
    }

    objSprite = new SpriteObject(
        "../Resource/Texture/Boss/Boss1.png",
        4,
        12
    );

    objSprite->SetAnimationLoop(
        0,
        0,
        2,
        200
    );

    objSprite->SetSize(500.0f, -500.0f);
}
void Boss::Update(float dt)
{
    Enemy::Update(dt);

    // boss logic
}

void Boss::TakeTurn()
{
    std::cout << "Boss takes turn!" << std::endl;
}