#include "EliteEnemy2.h"

EliteEnemy2::EliteEnemy2()
    : EliteEnemy(EliteType::Pattern)
{
    // ── Stats ────────────────────────────────────────────────────────────────
    maxHealth = 60;
    health    = maxHealth;
    damage    = 20;
    moveRange = 0;

    // ── Sprite ───────────────────────────────────────────────────────────────
    // TODO: replace with actual Elite2 texture path
    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy2.png", 4, 7);
    objSprite->SetAnimationLoop(0, 0, 6, 200);
    objSprite->SetSize(150.0f, -150.0f);

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    BuildPatterns();
    currentPatternIndex = currentPhase;
}

void EliteEnemy2::BuildPatterns()
{
    patterns.clear();

    // ── Pattern 1 ────────────────────────────────────────────────────────────
    // TODO: edit grid to the shape you want (O = origin / elite position)
    patterns.push_back(AttackPattern::fromGrid({
        "..X..",
        "..X..",
        "XXOXX",
        "..X..",
        "..X..",
    }, 'X'));

    // ── Pattern 2 ────────────────────────────────────────────────────────────
    patterns.push_back(AttackPattern::fromGrid({
        "X...X",
        ".X.X.",
        "..O..",
        ".X.X.",
        "X...X",
    }, 'X'));

    // ── Pattern 3 ────────────────────────────────────────────────────────────
    patterns.push_back(AttackPattern::fromGrid({
        "XXXXX",
        ".....",
        "X.O.X",
        ".....",
        "XXXXX",
    }, 'X'));

    // ── Pattern 4 ────────────────────────────────────────────────────────────
    patterns.push_back(AttackPattern::fromGrid({
        "..X..",
        "XXXXX",
        "XXOXX",
        "XXXXX",
        "..X..",
    }, 'X'));
}

void EliteEnemy2::Update(float dt)
{
    Enemy::Update(dt);
}

void EliteEnemy2::AdvancePattern()
{
    currentPhase        = (currentPhase + 1) % 4;
    currentPatternIndex = currentPhase;
}

bool EliteEnemy2::IsPlayerInPatternRange(int playerRow, int playerCol) const
{
    return patterns[currentPhase].hasOffset(
        playerRow - nowRow,
        playerCol - nowCol);
}

std::vector<std::pair<int, int>> EliteEnemy2::GetCurrentPatternTiles() const
{
    std::vector<std::pair<int, int>> tiles;
    auto cells = patterns[currentPhase].applyTo(nowRow, nowCol);
    for (auto& cell : cells)
        tiles.push_back({ cell.first.x, cell.first.y });
    return tiles;
}

std::vector<std::pair<int, int>> EliteEnemy2::GetCrossAttackTiles(
    int playerRow,
    int playerCol) const
{
    return {
        { playerRow,     playerCol     },   // player tile
        { playerRow - 1, playerCol     },   // N
        { playerRow + 1, playerCol     },   // S
        { playerRow,     playerCol - 1 },   // W
        { playerRow,     playerCol + 1 },   // E
    };
}
