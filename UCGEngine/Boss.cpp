#include "Boss.h"

Boss::Boss()
    : Enemy(EnemyType::C)
{
    maxHealth = 80;
    health = maxHealth;

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    damage = 5;
    moveRange = 0;

    if (objSprite) { delete objSprite; objSprite = nullptr; }

    objSprite = new SpriteObject("../Resource/Texture/Boss/Boss1.png", 8, 11);
    objSprite->SetAnimationLoop(0, 0, 5, 250);
    objSprite->SetSize(224.0f * 3, -160.0f * 3);
}

void Boss::Update(float dt)
{
    bool wasAttacking = isAttacking;
    bool wasTakingDamage = isTakingDamage;

    Enemy::Update(dt);

    // Enemy::Update resets idle to (0,0,2,200) � override with Boss idle
    if ((wasAttacking && !isAttacking) || (wasTakingDamage && !isTakingDamage))
    {
        if (objSprite)
            objSprite->SetAnimationLoop(0, 0, 5, 250);
    }
}

void Boss::RollAttackPattern()
{
    // Pattern center is applied at (nowRow, nowCol+2) in Level.cpp.
    // The combined transform (3×rot90CW + mirrorX) maps raw cell (dx,dy) → (dy,dx),
    // so a player at relative offset (relRow,relCol) is hit iff (relCol,relRow) exists
    // in the raw grid.
    //
    // Grid coverage after transform (boss fixed at nowRow=4, nowCol=0, center=(4,2)):
    //   1  – upper rows sweep (playerCol 0-2 only)
    //   2  – checkerboard: (playerRow+playerCol) even  → even==false
    //   3  – checkerboard: (playerRow+playerCol) odd   → even==true
    //   6  – left  half: rows 0-4, all cols
    //   7  – right half: rows 4-8, all cols
    //   8  – cross/plus centered on player (3-wide bars when HP < 50%)

    int relRow = playerRow - nowRow;  // +ve = player is right of boss center

    // Build the candidate pool based on current player position and state
    std::vector<int> candidates;

    // Checkerboard — guaranteed to land on the player's tile parity
    candidates.push_back(even ? 3 : 2);

    // Directional — attack the half the player is standing on
    candidates.push_back(relRow <= 0 ? 6 : 7);

    // Opposite directional for variety
    candidates.push_back(relRow > 0 ? 7 : 6);

    // Grid1 — upper-row sweep: only useful when player is in top visual rows (col 0-2)
    if (playerCol <= 2)
        candidates.push_back(1);

    // Cross attack: enters pool only when the last attack was NOT cross
    if (!lastWasCross)
        candidates.push_back(8);

    // Summon (choice 9): re-enters pool after ≥3 other skills have been used — not guaranteed
    if (skillsSinceLastSummon >= 3)
        candidates.push_back(9);

    attackPatternChoice = candidates[rand() % (int)candidates.size()];

    // Maintain the no-double-cross rule
    lastWasCross = (attackPatternChoice == 8);

    // Track how many non-summon skills have been used since the last summon
    if (attackPatternChoice == 9)
        skillsSinceLastSummon = 0;
    else
        skillsSinceLastSummon++;

    std::cout << "[Boss] Pattern " << attackPatternChoice
              << " | player(" << playerRow << "," << playerCol << ")"
              << " relRow=" << relRow
              << " even=" << even
              << " skillsSinceLastSummon=" << skillsSinceLastSummon << std::endl;
}

void Boss::PlayAttackAnimation(glm::vec3 playerPos)
{
    // Pattern already rolled when preparing � don't roll again here
    if (!objSprite) return;

    switch (attackPatternChoice)
    {
    case 1:
        // Grid 1 — upper row sweep
        objSprite->SetAnimationOnce(5, 0, 11, 150);
        attackDuration = 11 * 0.150f;
        break;

    case 2:
    case 3:
        // Grid 2/3 — checkerboard
        objSprite->SetAnimationOnce(4, 0, 11, 150);
        attackDuration = 11 * 0.150f;
        break;

    case 6:
    case 7:
    case 8:
        // Grid 6/7 half-field and cross attack
        objSprite->SetAnimationOnce(6, 0, 11, 150);
        attackDuration = 11 * 0.150f;
        break;

    case 9:
        // Summon enemy — get-damage anim (row 3, 9 frames) at 3x speed, looped 3 times
        objSprite->SetAnimationLoop(3, 0, 9, 50);   // 150ms / 3 = 50ms per frame
        attackDuration = 9 * 3 * 0.050f;            // 3 loops = 1.35 s
        break;

    default:
        objSprite->SetAnimationOnce(6, 0, 11, 150);
        attackDuration = 11 * 0.150f;
        break;
    }

    isAttacking = true;
    attackTimer = 0.0f;
}

void Boss::PlayBatterySummonAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(7, 0, 11, 150);
    attackDuration = 11 * 0.150f;
    isAttacking    = true;
    attackTimer    = 0.0f;
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
    if (attackPatternChoice == 6) chosen = &grid6;
    if (attackPatternChoice == 7) chosen = &grid7;

    // Summon (choice 9) — no tile attack, just spawns a minion via TryGetSummon().
    if (attackPatternChoice == 9)
        return AttackPattern{};

    // Cross attack (choice 8) — built dynamically from locked player position.
    // Pattern is applied at (nowRow, nowCol+2) = (4,2), so offsets are relative to that.
    if (attackPatternChoice == 8)
    {
        int drP = playerRow - nowRow;          // player row offset from pattern center
        int dcP = playerCol - (nowCol + 2);    // player col offset from pattern center (with +2 shift)

        bool enhanced = (health * 2 < maxHealth); // HP < 50% → 3-wide bars

        AttackPattern cross;

        if (!enhanced)
        {
            // Simple '+': full player row (all 5 cols) + full player col (all 9 rows)
            // Row sweep — all cols (dc: -2 to +2 covers grid cols 0-4)
            for (int dc = -2; dc <= 2; dc++)
                cross.addOffset(drP, dc, 1);

            // Col sweep — all rows (dr: -4 to +4 covers grid rows 0-8), skip player tile
            // (already added by row sweep above to avoid double damage)
            for (int dr = -4; dr <= 4; dr++)
            {
                if (dr == drP) continue; // already in row sweep
                cross.addOffset(dr, dcP, 1);
            }
        }
        else
        {
            // Enhanced '+': 3-tile thick bars
            // Horizontal band: playerRow-1/+0/+1, all cols
            for (int ddr = -1; ddr <= 1; ddr++)
                for (int dc = -2; dc <= 2; dc++)
                    cross.addOffset(drP + ddr, dc, 1);

            // Vertical band: all rows, playerCol-1/+0/+1
            // Skip rows already covered by the horizontal band to avoid duplicate damage
            for (int dr = -4; dr <= 4; dr++)
            {
                if (dr >= drP - 1 && dr <= drP + 1) continue; // already in horizontal band
                for (int ddc = -1; ddc <= 1; ddc++)
                    cross.addOffset(dr, dcP + ddc, 1);
            }
        }

        return cross;
    }

    AttackPattern rotated = AttackPattern::fromGrid(*chosen, 'X', 4, 3);
    int rotateTimes = 3;
    for (int i = 0; i < rotateTimes; i++)
        rotated = rotated.rotated90CW();
    rotated = rotated.mirroredX();
    return rotated;
    //////////////////////////////
}

int Boss::getAttackDamage() const
{
    // Per-pattern damage values (weaken multiplier still applied via base class logic)
    int baseDamage;
    switch (attackPatternChoice)
    {
    case 2:
    case 3:
        baseDamage = 10; // checkerboard patterns
        break;

    case 1:
    case 6:
    case 7:
        baseDamage = 20; // sweep / half-field patterns
        break;

    case 8:
        // Cross attack: 5 normally, 20 when enhanced (HP < 50% → 3-wide bars)
        baseDamage = (health * 2 < maxHealth) ? 20 : 5;
        break;

    case 9:
        baseDamage = 0; // summon skill — no direct tile damage
        break;

    default:
        baseDamage = damage; // fallback to base damage field
        break;
    }

    // Respect the weaken debuff the same way the base class does
    if (weakenTurns <= 0)
        return baseDamage;

    const float weakened = baseDamage * 0.75f;
    return std::max(0, static_cast<int>(std::ceil(weakened)));
}

int Boss::TryGetSummon()
{
    // Summon is now a normal rolled skill (choice 9) rather than HP-threshold based.
    return (attackPatternChoice == 9) ? 1 : 0;
}

bool Boss::ShouldSpawnBatteries()
{
    if (batteriesSummoned) return false;
    if (health * 2 <= maxHealth)      // HP ≤ 50%
    {
        batteriesSummoned = true;
        std::cout << "[Boss] HP ≤ 50% — spawning batteries!\n";
        return true;
    }
    return false;
}