#include "EliteEnemy1.h"

EliteEnemy1::EliteEnemy1()
    : EliteEnemy(EliteType::Row)
{
    // ── Stats ────────────────────────────────────────────────────────────────
    maxHealth = 80;
    health    = maxHealth;
    damage    = 15;
    moveRange = 1;

    // ── Sprite ───────────────────────────────────────────────────────────────
    // TODO: replace with actual Elite1 texture path
    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy1.png", 8, 12);
    objSprite->SetAnimationLoop(0, 0, 12, 200);
    objSprite->SetSize(150.0f, -150.0f);

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    // Row attack is built dynamically by GetRowAttackTiles() at runtime.
    // A dummy pattern is kept so getCurrentPattern() never indexes an empty vector.
    patterns.clear();
    patterns.push_back(AttackPattern::fromGrid({ "X" }, 'X'));
}

void EliteEnemy1::Update(float dt)
{
    Enemy::Update(dt);
}

std::vector<std::pair<int, int>> EliteEnemy1::GetRowAttackTiles(
    int gridStartCol,
    int gridEndCol) const
{
    std::vector<std::pair<int, int>> tiles;
    for (int c = gridStartCol; c < gridEndCol; ++c)
        tiles.push_back({ nowRow, c });
    return tiles;
}

bool EliteEnemy1::ShouldHealInstead(const Enemy* other) const
{
    return dynamic_cast<const EliteEnemy1*>(other) != nullptr;
}

bool EliteEnemy1::TryMoveTowardPlayer(
    int playerRow,
    int playerCol,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    const std::vector<Enemy*>& allEnemies,
    const std::function<bool(int, int)>& isWalkable,
    int& outR, int& outC)
{
    std::cout << "[Elite1] TryMove called — pos(" << nowRow << "," << nowCol
              << ") player(" << playerRow << "," << playerCol << ")\n";

    if (nowCol == playerCol)
        return false; // already on the correct column

    int targetR = nowRow;
    int targetC = nowCol + (playerCol > nowCol ? 1 : -1);

    if (targetC < gridStartCol || targetC >= gridEndCol)
        return false;

    if (!isWalkable(targetR, targetC))
        return false;

    for (auto* other : allEnemies)
    {
        if (other != this &&
            other->getNowRow() == targetR &&
            other->getNowCol() == targetC)
            return false;
    }

    outR = targetR;
    outC = targetC;
    return true;
}
