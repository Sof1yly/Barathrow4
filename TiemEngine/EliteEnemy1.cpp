#include "EliteEnemy1.h"

// Sprite sheet rows (adjust frame counts to match your actual asset):
static constexpr int E1_ROW_IDLE   = 0;
static constexpr int E1_ROW_WALK   = 0; // use idle row while moving
static constexpr int E1_ROW_ATTACK = 3;
static constexpr int E1_ROW_DAMAGE = 4;
static constexpr int E1_ROW_DEATH  = 5;

static constexpr int E1_FRAMES_IDLE   = 12;
static constexpr int E1_FRAMES_ATTACK = 4;
static constexpr int E1_FRAMES_DAMAGE = 4;
static constexpr int E1_FRAMES_DEATH  = 12;

EliteEnemy1::EliteEnemy1()
    : EliteEnemy(EliteType::Row)
{
    maxHealth  = 25;
    health     = maxHealth;
    damage     = 5;
    moveRange  = 1;
    countdown  = 3;
    spriteSize = 150.0f;

    if (objSprite) { delete objSprite; objSprite = nullptr; }
    objSprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy1.png", 8, 12);
    PlayIdleAnimation();

    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    patterns.clear();
    patterns.push_back(AttackPattern::fromGrid({ "X" }, 'X'));
}

// ── Idle ─────────────────────────────────────────────────────────────────────
void EliteEnemy1::PlayIdleAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationLoop(E1_ROW_IDLE, 0, E1_FRAMES_IDLE, 200);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);
}

// ── Attack ────────────────────────────────────────────────────────────────────
void EliteEnemy1::PlayAttackAnimation(glm::vec3 playerPos)
{
    if (!objSprite) return;

    // Update facing direction (left = default, right = X-flip)
    facingRight = (playerPos.x > objSprite->GetPosition().x);
    float sx = facingRight ? -spriteSize : spriteSize;
    objSprite->SetSize(sx, -spriteSize);

    objSprite->SetAnimationOnce(E1_ROW_ATTACK, 0, E1_FRAMES_ATTACK, 120);
    isAttacking  = true;
    attackTimer  = 0.0f;
}

// ── Damage ────────────────────────────────────────────────────────────────────
void EliteEnemy1::PlayDamageAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E1_ROW_DAMAGE, 0, E1_FRAMES_DAMAGE, 100);
}

// ── Death ─────────────────────────────────────────────────────────────────────
void EliteEnemy1::PlayDeathAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationOnce(E1_ROW_DEATH, 0, E1_FRAMES_DEATH, 120);
}

// ── Update ────────────────────────────────────────────────────────────────────
void EliteEnemy1::Update(float dt)
{
    if (!objSprite) return;

    // Death: wait for animation to finish before flagging the enemy as removed
    if (isDeadAnimating)
    {
        if (objSprite->IsFinished())
        {
            isDead         = true;
            isDeadAnimating = false;
        }
        UpdateMoveTween(dt);
        Enemy::UpdateTextPosition();
        return;
    }

    // Attack / damage: transition back to idle when the one-shot finishes
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

// ── Attack tile list ──────────────────────────────────────────────────────────
std::vector<std::pair<int, int>> EliteEnemy1::GetRowAttackTiles(
    int gridStartRow,
    int gridEndRow) const
{
    std::vector<std::pair<int, int>> tiles;
    for (int r = gridStartRow; r < gridEndRow; ++r)
        tiles.push_back({ r, nowCol });
    return tiles;
}

bool EliteEnemy1::ShouldHealInstead(const Enemy* other) const
{
    return dynamic_cast<const EliteEnemy1*>(other) != nullptr;
}

// ── Movement ──────────────────────────────────────────────────────────────────
bool EliteEnemy1::TryMoveTowardPlayer(
    int playerRow, int playerCol,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    const std::vector<Enemy*>& allEnemies,
    const std::function<bool(int, int)>& isWalkable,
    int& outR, int& outC)
{
    std::cout << "[Elite1] TryMove — pos(" << nowRow << "," << nowCol
              << ") player(" << playerRow << "," << playerCol << ")\n";

    if (moveCooldown > 0)
    {
        std::cout << "[Elite1] Move cooldown: " << moveCooldown << " turn(s) remaining.\n";
        moveCooldown--;
        return false;
    }

    if (nowCol == playerCol)
    {
        std::cout << "[Elite1] Aligned with player col " << playerCol << "\n";
        return false;
    }

    int targetR = nowRow;
    int targetC = nowCol + (playerCol > nowCol ? 1 : -1);

    if (targetC < gridStartCol || targetC >= gridEndCol) return false;
    if (!isWalkable(targetR, targetC))                   return false;

    for (auto* other : allEnemies)
    {
        if (other != this &&
            other->getNowRow() == targetR &&
            other->getNowCol() == targetC)
            return false;
    }

    moveCooldown = moveCooldownMax;
    outR = targetR;
    outC = targetC;
    return true;
}
