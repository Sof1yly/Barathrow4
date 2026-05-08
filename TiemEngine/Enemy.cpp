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
    }
    setNowPosition(8, 0);
    
}

void Enemy::setHealth(int h)
{
	health = h;
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
              << " turn(s). Total weaken turns: " << weakenTurns << std::endl;
    RefreshDebuffText();
}

void Enemy::decrementWeaken()
{
    if (weakenTurns > 0)
    {
        weakenTurns--;
        std::cout << "[Weaken] Enemy weaken decremented. Remaining: "
                  << weakenTurns << std::endl;
        RefreshDebuffText();
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
        hpText->SetPosition(glm::vec3(pos.x, pos.y + 80.0f, 100));

    if (debuffText)
        debuffText->SetPosition(glm::vec3(pos.x, pos.y + 30.0f, 200));
}

void Enemy::rotatePattern() {
    patterns[currentPatternIndex] = patterns[currentPatternIndex].rotated90CW();
}
void Enemy::Update(float dt)
{
    if (!objSprite) return;

    glm::vec3 pos = objSprite->GetPosition();

    // HP above enemy
    hpText->SetPosition(glm::vec3(pos.x, pos.y + 80, 200));

    // Corruption text below HP
    if (corruptText)
        corruptText->SetPosition(glm::vec3(pos.x, pos.y + 55, 200));

    if (debuffText)
        debuffText->SetPosition(glm::vec3(pos.x, pos.y + 30, 200));

	//Reset to idle after damage/attack animation
    if (isTakingDamage)
    {
        damageTimer += dt;

        if (damageTimer >= damageDuration)
        {
            objSprite->SetAnimationLoop(0, 0, 2, 200);

            isTakingDamage = false;
            damageTimer = 0.0f;
        }
    }
    if (isAttacking)
    {
        attackTimer += dt;

        if (attackTimer >= attackDuration)
        {
            objSprite->SetAnimationLoop(0, 0, 2, 200);

            isAttacking = false;
            attackTimer = 0.0f;
        }
    }
    if (isMoving)
    {
        moveTimer += dt;
        float t = moveTimer / (moveDuration);
        t = std::min(t, 1.0f);

        glm::vec3 newPos = moveStart + (moveTarget - moveStart) * t;
        objSprite->SetPosition(newPos);

        if (t >= 1.0f)
        {
            objSprite->SetPosition(moveTarget);
            isMoving = false;

            objSprite->SetAnimationLoop(0, 0, 2, 200);
        }
    }

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

void Enemy::RefreshDebuffText()
{
    if (!debuffText) return;

    if (delayTurns <= 0 && weakenTurns <= 0)
    {
        debuffText->SetSize(0, 0);
        return;
    }

    SDL_Color yellow = { 255, 255, 0 };
    std::string txt = "DEBUFF";

    if (delayTurns > 0)
        txt += " DLY:" + std::to_string(delayTurns);
    if (weakenTurns > 0)
        txt += " WEK:" + std::to_string(weakenTurns);

    debuffText->LoadText(txt, yellow, 18);
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
        {
			cout << "Enemy attacks right!" << endl;
            objSprite->SetAnimationLoop(2, 7, 6, 200);
        }
        else
        {
			cout << "Enemy attacks left!" << endl;
            objSprite->SetAnimationLoop(2, 0, 6, 200);
        }
    }
    else
    {
        if (dy > 0)
        {
			cout << "Enemy attacks down!" << endl;
            objSprite->SetAnimationLoop(1, 7, 6, 200);
        }
        else
        {
			cout << "Enemy attacks up!" << endl;
            objSprite->SetAnimationLoop(1, 0, 6, 200);
        }
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
    if (debuffText) {
        delete debuffText;
        debuffText = nullptr;
    }
    if (hpText) {
        delete hpText;
        hpText = nullptr;
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
        // horizontal

        if (dx > 0)
        {
            // RIGHT
            rotateTimes = 2;
        }
        else
        {
            // LEFT
            rotateTimes = 0;
        }
    }
    else
    {
        // vertical

        if (dy < 0)
        {
            // UP
            rotateTimes = 3;
        }
        else
        {
            // DOWN
            rotateTimes = 1;
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

    objSprite->SetAnimationLoop(0, 0, 2, 150);
}