#include "Boss.h"

Boss::Boss()
    : Enemy(EnemyType::C)
{
    maxHealth = 50;
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
    // Pattern already rolled when preparing — don't roll again here
    if (!objSprite) return;

    switch (attackPatternChoice)
    {
    case 1: objSprite->SetAnimationOnce(2, 0, 11, 150); attackDuration = 11 * 0.150f; break;
    case 2: objSprite->SetAnimationOnce(2, 0, 11, 150); attackDuration = 11 * 0.150f; break;
    case 3: objSprite->SetAnimationOnce(2, 0, 11, 150); attackDuration = 11 * 0.150f; break;
    }

    isAttacking = true;
    attackTimer = 0.0f;
}
void Boss::setPreparingAttack(bool value)
{
    if (value && !preparingAttack)
        RollAttackPattern();

    Enemy::setPreparingAttack(value);
}

void Boss::getDamage(int damage)
{
    Enemy::getDamage(damage);

    if (!objSprite) return;

    if (isDead)
    {
        objSprite->SetAnimationLoop(1, 0, 7, 150);//die
    }
    else
    {
        objSprite->SetAnimationOnce(3, 0, 9, 150);//get damage
        damageDuration = 9 * 0.150f;
    }
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
    std::vector<std::string> grid1 =
    {//hit up
        "ooooooooo",
        "XXXXXXXXX",
        "XXXXXXXXX",
        "XXXXXXXXX",
        "ooooooooo",
        "ooooooooo",
        "ooooooooo",
    };

    std::vector<std::string> grid2 =
    {//hit all odd
        "ooooooooo",
        "XoooooooX",
        "oXoooooXo",
        "XoXoXoXoX",
        "oXoXoXoXo",
        "XoXoXoXoX",
        "ooooooooo",
    };

    std::vector<std::string> grid3 =
    {//hit all even
        "ooooooooo",
        "oXoooooXo",
        "XoooooooX",
        "oXoXoXoXo",
        "XoXoXoXoX",
        "oXoXoXoXo",
        "ooooooooo",
    };
    std::vector<std::string> grid4 =
    {//lock player
        "ooooooooo",
        "ooooXoooo",
        "oooXXXooo",
        "ooXXXXXoo",
        "oooXXXooo",
        "ooooXoooo",
        "ooooooooo",
    };
    std::vector<std::string> grid5 =
    {//lock player
        "ooooooooo",
        "oooXXXooo",
        "oooXXXooo",
        "oooXXXooo",
        "oooXXXooo",
        "oooXXXooo",
        "ooooooooo",
    };
    std::vector<std::string> grid6 =
    {//hit all left
        "ooooooooo",
        "XXXXXoooo",
        "XXXXXoooo",
        "XXXXXoooo",
        "XXXXXoooo",
        "XXXXXoooo",
        "ooooooooo",
    };
    std::vector<std::string> grid7 =
    {//hit all right
        "ooooooooo",
        "ooooXXXXX",
        "ooooXXXXX",
        "ooooXXXXX",
        "ooooXXXXX",
        "ooooXXXXX",
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

int Boss::TryGetSummon()
{
    int count = 0;
    float hpPct = (float)health / maxHealth;

    if (!summoned66 && hpPct <= 0.66f)
    {
        summoned66 = true;
        count++;
    }
    if (!summoned33 && hpPct <= 0.33f)
    {
        summoned33 = true;
        count++;
    }
    return count;
}