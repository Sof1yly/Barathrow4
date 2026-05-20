#include "EliteEnemy3.h"

static constexpr int E3_ROW_IDLE   = 0;
static constexpr int E3_ROW_ATTACK = 1;
static constexpr int E3_ROW_DAMAGE = 2;
static constexpr int E3_ROW_DEATH  = 3;

static constexpr int E3_FRAMES_IDLE   = 6;
static constexpr int E3_FRAMES_ATTACK = 7;
static constexpr int E3_FRAMES_DAMAGE = 3;
static constexpr int E3_FRAMES_DEATH  = 7;

EliteEnemy3::EliteEnemy3()
    : EliteEnemy(EliteType::Pattern)
{
    maxHealth  = 45;
    health     = maxHealth;
    damage     = 20;
    moveRange  = 4;
    spriteSize = 150.0f;

    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy4.png", 8, 12);
    PlayIdleAnimation();

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    BuildPatterns();
    currentPatternIndex = patternPhase;
}

// ── Pattern construction ──────────────────────────────────────────────────────
void EliteEnemy3::BuildPatterns()
{
    patterns.clear();

    // Pattern 0: wide directional sweep — rotates toward player.
    // Base direction points toward decreasing col (upward on screen).
    //   ".......",
    //   ".XX....",
    //   "XXXO...",
    //   ".XX....",
    //   ".......",
    patterns.push_back(AttackPattern::fromGrid({
        ".......",
        ".XX....",
        "XXXO...",
        ".XX....",
        ".......",
    }, 'X'));

    // Pattern 1: straight-line charge — rotates toward player.
    // Enemy moves to the farthest attacked tile after firing (or deals x2 if player is there).
    //   ".......",
    //   ".......",
    //   "XXXO...",
    //   ".......",
    //   ".......",
    patterns.push_back(AttackPattern::fromGrid({
        ".......",
        ".......",
        "XXXO...",
        ".......",
        ".......",
    }, 'X'));
}

// ── Pattern cycling ───────────────────────────────────────────────────────────
void EliteEnemy3::AdvancePattern()
{
    patternPhase        = (patternPhase + 1) % 2;
    currentPatternIndex = patternPhase;
}

// ── Tile helpers ──────────────────────────────────────────────────────────────
std::vector<std::pair<int, int>> EliteEnemy3::GetPatternTiles(int playerRow, int playerCol) const
{
    AttackPattern rotated = GetRotatedPatternTowardPlayer(playerRow, playerCol);
    auto cells = rotated.applyTo(nowRow, nowCol);

    std::vector<std::pair<int, int>> tiles;
    tiles.reserve(cells.size());
    for (auto& cell : cells)
        tiles.push_back({ cell.first.x, cell.first.y });
    return tiles;
}

std::pair<int, int> EliteEnemy3::GetLineEndCell(int playerRow, int playerCol) const
{
    AttackPattern rotated = GetRotatedPatternTowardPlayer(playerRow, playerCol);
    auto cells = rotated.applyTo(nowRow, nowCol);

    int maxDist = -1;
    std::pair<int, int> farthest = { nowRow, nowCol };
    for (auto& cell : cells)
    {
        int dist = abs(cell.first.x - nowRow) + abs(cell.first.y - nowCol);
        if (dist > maxDist)
        {
            maxDist  = dist;
            farthest = { cell.first.x, cell.first.y };
        }
    }
    return farthest;
}

// ── Idle ──────────────────────────────────────────────────────────────────────
void EliteEnemy3::PlayIdleAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationLoop(E3_ROW_IDLE, 0, E3_FRAMES_IDLE, 200);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);
}

// ── Attack ────────────────────────────────────────────────────────────────────
void EliteEnemy3::PlayAttackAnimation(glm::vec3 playerPos)
{
    if (!objSprite) return;
    facingRight = (playerPos.x > objSprite->GetPosition().x);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);
    objSprite->SetAnimationOnce(E3_ROW_ATTACK, 0, E3_FRAMES_ATTACK, 120);
    isAttacking = true;
    attackTimer = 0.0f;
}

// ── Damage ────────────────────────────────────────────────────────────────────
void EliteEnemy3::PlayDamageAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E3_ROW_DAMAGE, 0, E3_FRAMES_DAMAGE, 100);
}

// ── Death ─────────────────────────────────────────────────────────────────────
void EliteEnemy3::PlayDeathAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E3_ROW_DEATH, 0, E3_FRAMES_DEATH, 120);
}

// ── Pattern selection ─────────────────────────────────────────────────────────
void EliteEnemy3::SelectPattern(int playerRow, int playerCol)
{
    bool aligned = (playerRow == nowRow) || (playerCol == nowCol);
    patternPhase        = aligned ? (rand() % 2) : 0;
    currentPatternIndex = patternPhase;
}

// ── Update ────────────────────────────────────────────────────────────────────
void EliteEnemy3::Update(float dt)
{
    if (!objSprite) return;

    if (isDeadAnimating)
    {
        if (objSprite->IsFinished())
        {
            isDead          = true;
            isDeadAnimating = false;
        }
        UpdateMoveTween(dt);
        Enemy::UpdateTextPosition();
        return;
    }

    if (isAttacking && objSprite->IsFinished())
    {
        isAttacking = false;
        attackTimer = 0.0f;
        PlayIdleAnimation();
    }
    if (isTakingDamage && objSprite->IsFinished())
    {
        isTakingDamage = false;
        damageTimer    = 0.0f;
        PlayIdleAnimation();
    }

    UpdateMoveTween(dt);
    Enemy::UpdateTextPosition();
}
