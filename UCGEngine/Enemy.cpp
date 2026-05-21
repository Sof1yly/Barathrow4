#include "Enemy.h"
#include <iostream>
#include <cmath>
#include <queue>
#include "GridConfig.h"

std::string EnemyTypeToString(Enemy::EnemyType type)
{
    switch (type)
    {
    case Enemy::EnemyType::A: return "A";
    case Enemy::EnemyType::B: return "B";
    case Enemy::EnemyType::C: return "C";
    case Enemy::EnemyType::D: return "D";
    case Enemy::EnemyType::E: return "E";
    case Enemy::EnemyType::F: return "F";
    case Enemy::EnemyType::G: return "G";
    case Enemy::EnemyType::H: return "H";
    case Enemy::EnemyType::I: return "I";
    default: return "A";
    }
}

Enemy::Enemy(EnemyType type)
{
    this->type = type;

    std::string typeStr = EnemyTypeToString(type);
    EnemyData d = EnemyDatabase::GetData(typeStr);

    maxHealth = d.hp;
    damage = d.atk;
	moveRange = d.mov;
    countdown = d.countdown;
	attackInc = d.attackIncrement;

    std::string patternName = d.pattern;
    std::vector<std::string> grid = EnemyLoadPattern::GetPattern(patternName);

    if (!grid.empty())
    {
        patterns.clear();
        patterns.push_back(AttackPattern::fromGrid(grid, 'X'));
    }
    else
    {
        std::cout << "[ERROR] Pattern not found: " << patternName << std::endl;

        // fallback (prevent crash)
        patterns = {
            AttackPattern::fromGrid({
                "X"
            }, 'X')
        };
    }


    health = maxHealth;
    // TEXT 
    hpText = new TextObject();
    SDL_Color white = { 255,255,255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);

    corruptText = new TextObject();
    corruptText->SetSize(0, 0);

    debuffText = new TextObject();
    debuffText->SetSize(0, 0);

    weakenText = new TextObject();
    weakenText->SetSize(0, 0);

    countdownIcon = new ImageObject();
    countdownIcon->SetTexture("../Resource/Texture/Vfx/debuffT.png");
    countdownIcon->SetSize(0.0f, 0.0f); // hidden until preparingAttack

    countdownText = new TextObject();
    countdownText->SetSize(0, 0);

	//Sprite Create
    switch (type)
    {
    case EnemyType::A:
    default:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy1.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;

    case EnemyType::B:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy2.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
    case EnemyType::C:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy3.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
    case EnemyType::D:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy4.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
    case EnemyType::E:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy5.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
    case EnemyType::F:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy6.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
    case EnemyType::G:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy7.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(200.0f, -200.0f);
        break;
	case EnemyType::H:
        objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy8.png", 4, 12);
        objSprite->SetAnimationLoop(
            0,   // start frame
            0,   // row
            2,   // end frame
            200  // ms per frame
        );
        objSprite->SetSize(120.0f, -120.0f);
		break;
    }

    setNowPosition(8, 0);
    
}

void Enemy::setHealth(int h)
{
	health = h;
    if (hpText)
    {
        SDL_Color white = { 255, 255, 255 };
        hpText->LoadText("HP: " + std::to_string(health), white, 24);
    }
}

void Enemy::getDamage(int damage)
{
	int total = damage + corruptionStacks;
	health -= total;
    if (health <= 0) {
        health = 0;
        isDead = true;
    }
    if (health == 0&& objSprite) {
        objSprite->SetAnimationLoop(
            3,
            0,
            6,
            100
        );

        isTakingDamage = true;
        damageTimer = 0.0f;
    }else
    {
        objSprite->SetAnimationLoop(
            3,
            0, 
            3,
            100 
        );

        isTakingDamage = true;
        damageTimer = 0.0f;
    }

	if (corruptionStacks > 0)
		std::cout << "[Corrupt] Enemy takes " << corruptionStacks << " extra damage (total " << total << ")" << std::endl;

	SDL_Color white = { 255, 255, 255 };
	hpText->LoadText("HP: " + std::to_string(health), white, 24);
}

bool Enemy::TryMoveTowardPlayer(
    int playerRow,
    int playerCol,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    const std::vector<Enemy*>& allEnemies,
    const std::function<bool(int, int)>& isWalkable,
    int& outR, int& outC)
{
    struct Node
    {
        int r;
        int c;
    };

    auto IsBlockedByEnemy = [&](int r, int c) -> bool
        {
            for (auto* other : allEnemies)
            {
                if (other != this &&
                    other->getNowRow() == r &&
                    other->getNowCol() == c)
                {
                    return true;
                }
            }
            return false;
        };

    auto CanWalk = [&](int r, int c) -> bool
        {
            if (r < gridStartRow || r >= gridEndRow ||
                c < gridStartCol || c >= gridEndCol)
            {
                return false;
            }

            if (!isWalkable(r, c))
            {
                return false;
            }

            if (IsBlockedByEnemy(r, c))
            {
                return false;
            }

            return true;
        };

    int startR = getNowRow();
    int startC = getNowCol();

    static const int dr[4] = { 1, -1, 0, 0 };
    static const int dc[4] = { 0, 0, 1, -1 };

    bool visited[GRID_ROWS][GRID_COLS] = {};
    Node parent[GRID_ROWS][GRID_COLS];

    std::queue<Node> q;

    q.push({ startR, startC });
    visited[startR][startC] = true;

    bool foundPlayer = false;

    int bestR = startR;
    int bestC = startC;

    int bestDist =
        abs(startR - playerRow) +
        abs(startC - playerCol);

    while (!q.empty())
    {
        Node cur = q.front();
        q.pop();

        int dist =
            abs(cur.r - playerRow) +
            abs(cur.c - playerCol);

        // nearest reachable tile to player
        if (dist < bestDist)
        {
            bestDist = dist;
            bestR = cur.r;
            bestC = cur.c;
        }

        if (cur.r == playerRow &&
            cur.c == playerCol)
        {
            foundPlayer = true;
            bestR = cur.r;
            bestC = cur.c;
            break;
        }

        for (int i = 0; i < 4; i++)
        {
            int nr = cur.r + dr[i];
            int nc = cur.c + dc[i];

            if (!CanWalk(nr, nc))
                continue;

            if (visited[nr][nc])
                continue;

            visited[nr][nc] = true;

            parent[nr][nc] = cur;

            q.push({ nr, nc });
        }
    }

    // no movement possible
    if (bestR == startR && bestC == startC)
    {
        return false;
    }

    int curR = bestR;
    int curC = bestC;

    while (true)
    {
        Node p = parent[curR][curC];

        if (p.r == startR &&
            p.c == startC)
        {
            outR = curR;
            outC = curC;
            return true;
        }

        curR = p.r;
        curC = p.c;
    }

    return false;
}

void Enemy::addWeaken(int turns)
{
    if (turns <= 0) return;
    weakenTurns += turns;
    std::cout << "[Weaken] Enemy weakened for +" << turns
              << " turn(s). Total: " << weakenTurns << std::endl;
    RefreshWeakenText();
}

void Enemy::decrementWeaken()
{
    if (weakenTurns > 0)
    {
        weakenTurns--;
        std::cout << "[Weaken] Enemy weaken decremented. Remaining: "
                  << weakenTurns << std::endl;
        RefreshWeakenText();
    }
}

int Enemy::getAttackDamage() const
{
    if (weakenTurns <= 0)
        return damage;

    const float weakenedDamage = damage * 0.75f;
    return std::max(0, static_cast<int>(std::ceil(weakenedDamage)));
}



void Enemy::addCorruption(int stacks)
{
	corruptionStacks += stacks;
	std::cout << "[Corrupt] Enemy corruption: +" << stacks << " (total " << corruptionStacks << ")" << std::endl;

	if (corruptText) {
		SDL_Color red = { 255, 0, 0 };
		corruptText->LoadText("COR: " + std::to_string(corruptionStacks), red, 20);
	}
}

void Enemy::UpdateTextPosition()
{
    if (!objSprite) return;

    glm::vec3 pos = objSprite->GetPosition();

    if (hpText)
        hpText->SetPosition(glm::vec3(pos.x, pos.y + 110.0f, 200));

    if (corruptText)
        corruptText->SetPosition(glm::vec3(pos.x, pos.y + 85.0f, 200));

    if (weakenText)
        weakenText->SetPosition(glm::vec3(pos.x, pos.y + 60.0f, 200));

    if (debuffText)
        debuffText->SetPosition(glm::vec3(pos.x, pos.y + 35.0f, 200));

    if (countdownIcon)
        countdownIcon->SetPosition(glm::vec3(pos.x + 55.0f, pos.y - 55.0f, 100.0f));

    if (countdownText)
        countdownText->SetPosition(glm::vec3(pos.x + 55.0f, pos.y - 55.0f, 105.0f));
}

void Enemy::rotatePattern() {
    patterns[currentPatternIndex] = patterns[currentPatternIndex].rotated90CW();
}
void Enemy::PlayIdleAnimation()
{
    if (!objSprite) return;
    objSprite->SetAnimationLoop(0, 0, 2, 200);
}

void Enemy::UpdateMoveTween(float dt)
{
    if (!isMoving) return;

    moveTimer += dt;
    float t = std::min(moveTimer / moveDuration, 1.0f);

    objSprite->SetPosition(moveStart + (moveTarget - moveStart) * t);

    if (t >= 1.0f)
    {
        objSprite->SetPosition(moveTarget);
        isMoving = false;
        PlayIdleAnimation();
    }
}

void Enemy::Update(float dt)
{
    if (!objSprite) return;

    glm::vec3 pos = objSprite->GetPosition();

    hpText->SetPosition(glm::vec3(pos.x, pos.y + 110, 200));

    if (corruptText)
        corruptText->SetPosition(glm::vec3(pos.x, pos.y + 85, 200));

    if (weakenText)
        weakenText->SetPosition(glm::vec3(pos.x, pos.y + 60, 200));

    if (debuffText)
        debuffText->SetPosition(glm::vec3(pos.x, pos.y + 35, 200));

    if (countdownIcon)
        countdownIcon->SetPosition(glm::vec3(pos.x + 55.0f, pos.y - 55.0f, 100.0f));

    if (countdownText)
        countdownText->SetPosition(glm::vec3(pos.x + 55.0f, pos.y - 55.0f, 105.0f));

    if (isTakingDamage)
    {
        damageTimer += dt;
        if (damageTimer >= damageDuration)
        {
            PlayIdleAnimation();
            isTakingDamage = false;
            damageTimer = 0.0f;
        }
    }
    if (isAttacking)
    {
        attackTimer += dt;
        if (attackTimer >= attackDuration)
        {
            PlayIdleAnimation();
            isAttacking = false;
            attackTimer = 0.0f;
        }
    }

    UpdateMoveTween(dt);
}

void Enemy::addDelay(int turns)
{
    delayTurns += turns;
    std::cout << "[Delay] Enemy delayed by " << turns << " turn(s). Total delay: " << delayTurns << std::endl;
    RefreshDebuffText();
}

bool Enemy::isDelayed() const
{
    return delayTurns > 0;
}

void Enemy::decrementDelay()
{
    if (delayTurns > 0) {
        delayTurns--;
        std::cout << "[Delay] Enemy delay decremented. Remaining: " << delayTurns << std::endl;
        RefreshDebuffText();
    }
}

void Enemy::addStun(int turns)
{
    if (turns <= 0) return;
    stunTurns += turns;
    std::cout << "[Stun] Enemy stunned for +" << turns << " turn(s). Total: " << stunTurns << std::endl;
    RefreshDebuffText();
}

bool Enemy::isStunned() const
{
    return stunTurns > 0;
}

void Enemy::decrementStun()
{
    if (stunTurns > 0) {
        stunTurns--;
        std::cout << "[Stun] Enemy stun decremented. Remaining: " << stunTurns << std::endl;
        RefreshDebuffText();
    }
}

bool Enemy::ShouldSkipTurn()
{
    if (stunTurns > 0) {
        stunTurns--;
        RefreshDebuffText();
        return true;
    }
    if (delayTurns > 0) {
        delayTurns--;
        RefreshDebuffText();
        return true;
    }
    return false;
}

void Enemy::RefreshDebuffText()
{
    if (!debuffText) return;
    if (stunTurns > 0)
    {
        SDL_Color cyan = { 100, 200, 255, 255 };
        debuffText->LoadText("STN: " + std::to_string(stunTurns), cyan, 20);
        return;
    }
    if (delayTurns > 0)
    {
        SDL_Color yellow = { 255, 255, 0, 255 };
        debuffText->LoadText("DLY: " + std::to_string(delayTurns), yellow, 20);
        return;
    }
    debuffText->SetSize(0, 0);
}

void Enemy::RefreshWeakenText()
{
    if (!weakenText) return;
    if (weakenTurns <= 0)
    {
        weakenText->SetSize(0, 0);
        return;
    }
    SDL_Color orange = { 255, 165, 0 };
    weakenText->LoadText("WEK: " + std::to_string(weakenTurns), orange, 20);
}

void Enemy::PlayAttackAnimation(glm::vec3 playerPos)
{
    if (!objSprite) return;

    glm::vec3 enemyPos = objSprite->GetPosition();

    float dx = playerPos.x - enemyPos.x;
    float dy = playerPos.y - enemyPos.y;

    if (abs(dx) > abs(dy))
    {
        if (dx > 0)
            objSprite->SetAnimationLoop(2, 7, 6, 200); // player right → attack right
        else
            objSprite->SetAnimationLoop(2, 0, 6, 200); // player left → attack left
    }
    else
    {
        if (dy > 0)
            objSprite->SetAnimationLoop(1, 7, 6, 200); // player above → attack up
        else
            objSprite->SetAnimationLoop(1, 0, 6, 200); // player below → attack down
    }
    isAttacking = true;
    attackTimer = 0.0f;
}

Enemy::~Enemy()
{
    if (objSprite) {
        delete objSprite;
        objSprite = nullptr;
    }
    if (corruptText) {
        delete corruptText;
        corruptText = nullptr;
    }
    if (weakenText) {
        delete weakenText;
        weakenText = nullptr;
    }
    if (debuffText) {
        delete debuffText;
        debuffText = nullptr;
    }
    if (hpText) {
        delete hpText;
        hpText = nullptr;
    }
    if (countdownIcon) {
        delete countdownIcon;
        countdownIcon = nullptr;
    }
    if (countdownText) {
        delete countdownText;
        countdownText = nullptr;
    }
}
void Enemy::SetWorldPosition(glm::vec3 pos)
{
    if (objSprite)
        objSprite->SetPosition(pos);

    UpdateTextPosition();
}

bool Enemy::isPreparingAttack() const
{
    return preparingAttack;
}

void Enemy::setPreparingAttack(bool value)
{
    preparingAttack = value;
    RefreshCountdownIcon();
}

void Enemy::RefreshCountdownIcon()
{
    if (!countdownIcon || !countdownText) return;

    if (!preparingAttack)
    {
        countdownIcon->SetSize(0.0f, 0.0f);
        countdownText->SetSize(0.0f, 0.0f);
        return;
    }

    bool warning = (countdownRemaning <= 0);
    countdownIcon->SetTexture(warning
        ? "../Resource/Texture/Vfx/attack.png"
        : "../Resource/Texture/Vfx/debuffT.png");
    countdownIcon->SetSize(55.0f, -55.0f);

    SDL_Color white = { 255, 255, 255, 255 };
    if (warning)
        countdownText->LoadText(std::to_string(getAttackDamage()), white, 22);
    else
        countdownText->LoadText(std::to_string(countdownRemaning), white, 22);
}

void Enemy::LockAttackPattern(int playerRow, int playerCol)
{
    lockedAttackPattern = GetRotatedPatternTowardPlayer(playerRow, playerCol);
    lockedPlayerRow = playerRow;
    lockedPlayerCol = playerCol;
}

AttackPattern Enemy::GetRotatedPatternTowardPlayer(
    int playerRow,
    int playerCol) const
{
    AttackPattern rotated = getCurrentPattern();

    int er = getNowRow();
    int ec = getNowCol();

    int rotateTimes = 0;

    int dx = playerRow - er;
    int dy = playerCol - ec;

    if (abs(dx) >= abs(dy))
    {
        

        if (dx > 0)
        {

            rotateTimes = 3;
        }
        else
        {

            rotateTimes = 1;
        }
    }
    else
    {

        if (dy < 0)
        {
            
            rotateTimes = 4;
        }
        else
        {
            
            rotateTimes = 2;
        }
    }

    for (int i = 0; i < rotateTimes; i++)
    {
        rotated = rotated.rotated90CW();
    }

    return rotated;
}

void Enemy::StartMove(glm::vec3 targetWorld)
{
    if (!objSprite) return;

    moveStart = objSprite->GetPosition();
    moveTarget = targetWorld;
    moveTimer = 0.0f;
    isMoving = true;

    PlayIdleAnimation();
}

std::vector<std::pair<int, int>> Enemy::GetOccupiedTiles() const
{
    return { { nowRow, nowCol } };
}