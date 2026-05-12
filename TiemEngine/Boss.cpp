#include "Boss.h"

Boss::Boss()
    : Enemy(EnemyType::C)
{
    maxHealth = 500;
    health = maxHealth;

    SDL_Color white = { 255,255,255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

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
        11
    );

    objSprite->SetAnimationLoop(
        0,
        0,
        5,
        250
    );

    objSprite->SetSize(224.0f*3, -160.0f*3 );
}
void Boss::Update(float dt)
{
    Enemy::Update(dt);
}

void Boss::TakeTurn()
{
    std::cout << "Boss Turn!" << std::endl;
}

std::vector<std::pair<int, int>> Boss::GetOccupiedTiles() const
{
    std::vector<std::pair<int, int>> tiles;

    for (int r = nowRow + hitboxTopOffset;
        r <= nowRow + hitboxBottomOffset;
        r++)
    {
        for (int c = nowCol + hitboxLeftOffset;
            c <= nowCol + hitboxRightOffset;
            c++)
        {
            tiles.push_back({ r, c });
        }
    }

    return tiles;
}

bool Boss::OccupiesTile(int row, int col) const
{
    for (const auto& tile : GetOccupiedTiles())
    {
        if (tile.first == row &&
            tile.second == col)
        {
            return true;
        }
    }

    return false;
}