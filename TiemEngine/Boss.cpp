#include "Boss.h"

Boss::Boss()
    : Enemy(EnemyType::C)
{
    maxHealth = 500;
    health = maxHealth;

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    damage = 25;
    moveRange = 0;

    if (objSprite) { delete objSprite; objSprite = nullptr; }

    objSprite = new SpriteObject("../Resource/Texture/Boss/Boss1.png", 4, 11);
    objSprite->SetAnimationLoop(0, 0, 5, 250);
    objSprite->SetSize(224.0f * 3, -160.0f * 3);
}

void Boss::Update(float dt)
{
    bool wasAttacking = isAttacking;
    bool wasTakingDamage = isTakingDamage;

    Enemy::Update(dt);

    // Enemy::Update resets idle to (0,0,2,200) — override with Boss idle
    if ((wasAttacking && !isAttacking) || (wasTakingDamage && !isTakingDamage))
    {
        if (objSprite)
            objSprite->SetAnimationLoop(0, 0, 5, 250);
    }
}

void Boss::RollAttackPattern()
{
    attackPatternChoice = (rand() % 3) + 1;
    std::cout << "[Boss] Attack pattern " << attackPatternChoice << std::endl;
}

void Boss::PlayAttackAnimation(glm::vec3 playerPos)
{
    RollAttackPattern();
    if (!objSprite) return;

    switch (attackPatternChoice)
    {
    case 1: //Attack animaiton
        objSprite->SetAnimationOnce(/* row */ 2, 0, /* frames */ 11, 150);
        attackDuration = 6 * 0.150f;
        break;
    case 2:
        objSprite->SetAnimationOnce(/* row */ 2, 0, /* frames */ 11, 150);
        attackDuration = 6 * 0.150f;
        break;
    case 3:
        objSprite->SetAnimationOnce(/* row */ 2, 0, /* frames */ 11, 150);
        attackDuration = 6 * 0.150f;
        break;
    }

    isAttacking = true;
    attackTimer = 0.0f;
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
    for (int r = nowRow + hitboxTopOffset; r <= nowRow + hitboxBottomOffset; r++)
        for (int c = nowCol + hitboxLeftOffset; c <= nowCol + hitboxRightOffset; c++)
            tiles.push_back({ r, c });
    return tiles;
}

bool Boss::OccupiesTile(int row, int col) const
{
    if (row >= nowRow + hitboxTopOffset && row <= nowRow + hitboxBottomOffset &&
        col >= nowCol + hitboxLeftOffset && col <= nowCol + hitboxRightOffset)
    {
        std::cout << "Boss occupies tile (" << row << ", " << col << ")\n";
        return true;
    }
    return false;
}

bool Boss::IsHitBy(const std::vector<std::pair<IVec2, int>>& cells)
{
    for (const auto& item : cells)
    {
        int row = item.first.x;
        int col = item.first.y;
        if (row >= nowRow + hitboxTopOffset && row <= nowRow + hitboxBottomOffset &&
            col >= nowCol + hitboxLeftOffset && col <= nowCol + hitboxRightOffset)
            return true;
    }
    return false;
}

AttackPattern Boss::GetRotatedPatternTowardPlayer(int playerRow, int playerCol) const
{
    // Pattern 1 — TODO: design your own grid
    std::vector<std::string> grid1 =
    {
        "ooooooooo",
        "XoooXoooo",
        "oooXXXooo",
        "XXXXXXXoo",
        "oooXXXooo",
        "XoooXoooo",
        "ooooooooo",
    };

    // Pattern 2 — TODO: design your own grid
    std::vector<std::string> grid2 =
    {
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
    };

    // Pattern 3 — TODO: design your own grid
    std::vector<std::string> grid3 =
    {
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
    };

    /////////////////////////////
    // Don't touch this line until end of comment
    const std::vector<std::string>* chosen = &grid1;
    if (attackPatternChoice == 2) chosen = &grid2;
    if (attackPatternChoice == 3) chosen = &grid3;

    AttackPattern rotated = AttackPattern::fromGrid(*chosen, 'X', 4, 3);
    int rotateTimes = 3;
    for (int i = 0; i < rotateTimes; i++)
        rotated = rotated.rotated90CW();
    rotated = rotated.mirroredX();
    return rotated;
    //////////////////////////////
}