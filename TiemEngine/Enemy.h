#pragma once
#include "SpriteObject.h"
#include "AttackPattern.h"
#include "TextObject.h"
#include "EnemyData.h"
#include "EnemyLoadPattern.h"
#include <functional>
#include <iostream>

class Enemy
{
public:

    enum class EnemyType
    {
        A, B, C, D, E, F, G, H, I
    };

public:

    Enemy(EnemyType type);

    virtual ~Enemy();

    virtual void Update(float dt);

    virtual bool CanMove() const
    {
        return true;
    }

    virtual void TakeTurn()
    {
    }

    AttackPattern getCurrentPattern() const
    {
        return patterns[currentPatternIndex];
    }

    void rotatePattern();

    void setHealth(int h);

    void setNowPosition(int row, int col)
    {
        nowRow = row;
        nowCol = col;
    }

    int getNowRow() const { return nowRow; }
    int getNowCol() const { return nowCol; }

    int getHealth() const { return health; }

    virtual void getDamage(int damage);

    virtual bool TryMoveTowardPlayer(
        int playerRow,
        int playerCol,
        int gridStartRow,
        int gridEndRow,
        int gridStartCol,
        int gridEndCol,
        const std::vector<Enemy*>& allEnemies,
        const std::function<bool(int, int)>& isWalkable,
        int& outR,
        int& outC
    );

    void setObject(SpriteObject* obj)
    {
        objSprite = obj;
    }

    SpriteObject* getObject()
    {
        return objSprite;
    }

    TextObject* getHPText()
    {
        return hpText;
    }

    TextObject* getCorruptText()
    {
        return corruptText;
    }

    TextObject* getDebuffText()
    {
        return debuffText;
    }

    void showAttackText()
    {
        attackTextTimer = 1.0f;
    }

    void UpdateTextPosition();

    void SetWorldPosition(glm::vec3 pos);

    void addDelay(int turns);

    bool isDelayed() const;

    int getDelayTurns() const
    {
        return delayTurns;
    }

    void decrementDelay();

    virtual void PlayAttackAnimation(glm::vec3 playerPos);

    void addCorruption(int stacks);

    int getCorruption() const
    {
        return corruptionStacks;
    }

    void addWeaken(int turns);

    int getWeakenTurns() const
    {
        return weakenTurns;
    }

    void decrementWeaken();

    int getAttackDamage() const;

    bool isPreparingAttack() const;

    virtual void setPreparingAttack(bool value);

    void LockAttackPattern(int playerRow, int playerCol);
    const AttackPattern& getLockedAttackPattern() const { return lockedAttackPattern; }
    int getLockedPlayerRow() const { return lockedPlayerRow; }
    int getLockedPlayerCol() const { return lockedPlayerCol; }

    bool getIsDead() const
    {
        return isDead;
    }

    void markDead()
    {
        isDead = true;
    }

    bool getIsMoving() const
    {
        return isMoving;
    }

    int getMoveRange() const
    {
        return moveRange;
    }

    int getCountDown() const
    {
        return countdown;
    }

    void setCountDownR()
    {
        countdownRemaning = countdown;
    }

    void decreaseCountDownR()
    {
        if (countdownRemaning > 0)
            countdownRemaning--;
    }

    int getCountDownR() const
    {
        return countdownRemaning;
    }

    void addDamage()
    {
        damage = damage + attackInc;
    }

    virtual AttackPattern GetRotatedPatternTowardPlayer(
        int playerRow,
        int playerCol) const;

    void StartMove(glm::vec3 targetWorld);

    int highlightIndex;

    int stepsRemaining = 0;

    virtual bool OccupiesTile(int row, int col) const
    {
        return nowRow == row &&
            nowCol == col;
    }
    virtual std::vector<std::pair<int, int>> GetOccupiedTiles() const;

    int getMaxHealth() const { return maxHealth; }


protected:

    EnemyType type;

    int maxHealth = 10;

    int health = maxHealth;

    int damage = 1;

    int moveRange = 1;

    int countdown = 0;

    int countdownRemaning = 0;

    int attackInc = 0;

    int nowRow = 0;

    int nowCol = 0;

    bool isMoving = false;

    bool preparingAttack = false;

    bool isDead = false;

    bool isTakingDamage = false;

    bool isAttacking = false;

    SpriteObject* objSprite = nullptr;

    std::vector<AttackPattern> patterns;

    int currentPatternIndex = 0;

    AttackPattern lockedAttackPattern;
    int lockedPlayerRow = 0;
    int lockedPlayerCol = 0;

    TextObject* hpText = nullptr;

    TextObject* corruptText = nullptr;

    TextObject* debuffText = nullptr;

    float moveTimer = 0.0f;

    float moveDuration = 0.95f;

    float damageTimer = 0.0f;

    float damageDuration = 2.0f;

    float attackTimer = 0.0f;

    float attackDuration = 1.0f;

    float attackTextTimer = 0.0f;

    int delayTurns = 0;

    int corruptionStacks = 0;

    int weakenTurns = 0;

private:

    void RefreshDebuffText();

    glm::vec3 moveStart;

    glm::vec3 moveTarget;
    
};