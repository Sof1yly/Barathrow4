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

bool Boss::OccupiesTile(int row, int col) const
{
    int startRow = nowRow + hitboxTopOffset;
    int endRow = nowRow + hitboxBottomOffset;

    int startCol = nowCol + hitboxLeftOffset;
    int endCol = nowCol + hitboxRightOffset;

    return (
        row >= startRow &&
        row <= endRow &&
        col >= startCol &&
        col <= endCol
        );
}

std::vector<std::pair<int, int>>
Boss::GetOccupiedTiles() const
{
    std::vector<std::pair<int, int>> tiles;

    int startRow = nowRow + hitboxTopOffset;
    int endRow = nowRow + hitboxBottomOffset;

    int startCol = nowCol + hitboxLeftOffset;
    int endCol = nowCol + hitboxRightOffset;

    for (int r = startRow; r <= endRow; r++)
    {
        for (int c = startCol; c <= endCol; c++)
        {
            tiles.push_back({ r, c });
        }
    }

    return tiles;
}