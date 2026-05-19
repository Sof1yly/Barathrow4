#include "EliteEnemy2.h"

// Sprite sheet rows (adjust frame counts to match your actual asset):
static constexpr int E2_ROW_IDLE   = 0;
static constexpr int E2_ROW_ATTACK = 1;
static constexpr int E2_ROW_DAMAGE = 2;
static constexpr int E2_ROW_DEATH  = 3;

static constexpr int E2_FRAMES_IDLE   = 6;
static constexpr int E2_FRAMES_ATTACK = 7;
static constexpr int E2_FRAMES_DAMAGE = 3;
static constexpr int E2_FRAMES_DEATH  = 7;

EliteEnemy2::EliteEnemy2()
    : EliteEnemy(EliteType::Pattern)
{
    maxHealth = 60;
    health    = maxHealth;
    damage    = 20;
    moveRange = 0;
    spriteSize = 150.0f;

    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy2.png", 4, 7);
    PlayIdleAnimation();

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    BuildPatterns();
    currentPatternIndex = currentPhase;
}

// ── Idle ─────────────────────────────────────────────────────────────────────
void EliteEnemy2::PlayIdleAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationLoop(E2_ROW_IDLE, 0, E2_FRAMES_IDLE, 200);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);
}

// ── Attack ────────────────────────────────────────────────────────────────────
void EliteEnemy2::PlayAttackAnimation(glm::vec3 playerPos)
{
    if (!objSprite) return;

    facingRight = (playerPos.x > objSprite->GetPosition().x);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);

    objSprite->SetAnimationOnce(E2_ROW_ATTACK, 0, E2_FRAMES_ATTACK, 120);
    isAttacking = true;
    attackTimer = 0.0f;
}

// ── Damage ────────────────────────────────────────────────────────────────────
void EliteEnemy2::PlayDamageAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E2_ROW_DAMAGE, 0, E2_FRAMES_DAMAGE, 100);
}

// ── Death ─────────────────────────────────────────────────────────────────────
void EliteEnemy2::PlayDeathAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E2_ROW_DEATH, 0, E2_FRAMES_DEATH, 120);
}

// ── Update ────────────────────────────────────────────────────────────────────
void EliteEnemy2::Update(float dt)
{
    if (!objSprite) return;

    if (isDeadAnimating)
    {
        if (objSprite->IsFinished())
        {
            isDead          = true;
            isDeadAnimating = false;
        }
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

// ── Pattern logic ─────────────────────────────────────────────────────────────
void EliteEnemy2::BuildPatterns()
{
    patterns.clear();

    patterns.push_back(AttackPattern::fromGrid({
        ".......",
        ".......",
        "..XXX..",
        "..XOX..",
        "..XXX..",
        ".......",
		".......",
    }, 'X'));

    patterns.push_back(AttackPattern::fromGrid({
		".......",
        "...X...",
        "..XXX..",
        ".XXOXX.",
        "..XXX..",
        "...X...",
		".......",
    }, 'X'));

    patterns.push_back(AttackPattern::fromGrid({
		".......",
        ".XXXXX.",
        ".XXXXX.",
        ".XXOXX.",
        ".XXXXX.",
        ".XXXXX.",
		".......",
    }, 'X'));

    patterns.push_back(AttackPattern::fromGrid({
        "XXXXXXX",
        "XXXXXXX",
        "XXXXXXX",
        "XXXOXXX",
        "XXXXXXX",
        "XXXXXXX",
        "XXXXXXX",
    }, 'X'));
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
    int playerRow, int playerCol) const
{
    return {
        { playerRow,     playerCol     },
        { playerRow - 1, playerCol     },
        { playerRow + 1, playerCol     },
        { playerRow,     playerCol - 1 },
        { playerRow,     playerCol + 1 },
    };
}
