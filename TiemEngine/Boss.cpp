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
    std::cout << "======================================" << std::endl;
    std::cout << "==========    Boss Turn   ============" << std::endl;
    std::cout << "======================================" << std::endl;
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
    const int top = nowRow + hitboxTopOffset;
    const int bottom = nowRow + hitboxBottomOffset;
    const int left = nowCol + hitboxLeftOffset;
    const int right = nowCol + hitboxRightOffset;

    if (row >= top && row <= bottom &&
        col >= left && col <= right)
    {
        cout << "Boss occupies tile (" << row << ", " << col << ")\n";
        return true;
    }

    return false;
}
bool Boss::IsHitBy(const std::vector<std::pair<IVec2, int>>& cells)
{
    int topRow = nowRow + hitboxTopOffset;
    int bottomRow = nowRow + hitboxBottomOffset;
    int leftCol = nowCol + hitboxLeftOffset;
    int rightCol = nowCol + hitboxRightOffset;

    for (const auto& item : cells)
    {
        int row = item.first.x;
        int col = item.first.y;

        if (row >= topRow && row <= bottomRow &&
            col >= leftCol && col <= rightCol)
        {
            return true;
        }
    }

    return false;
}
AttackPattern Boss::GetRotatedPatternTowardPlayer(int playerRow, int playerCol) const
{
    // Build a 5x9 grid where every cell is marked as an attack tile
    std::vector<std::string> fullGrid = {
        "0XXXXXXXX",
        "XXXXXXXXX",
        "XXXXXXXXX",
        "XXXXXXXXX",
        "XXXXXXXXX"
    };

    return AttackPattern::fromGrid(fullGrid, 'X');
}