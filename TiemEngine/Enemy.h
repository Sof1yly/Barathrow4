#pragma once
#include "SpriteObject.h"
#include "AttackPattern.h"
#include "TextObject.h"
#include "EnemyData.h"
#include "EnemyLoadPattern.h"

class Enemy {
public:
    enum class EnemyType {
        A, B, C, D, E, F, G, H, I

    };

    Enemy(EnemyType type);
    ~Enemy();

    AttackPattern getCurrentPattern() const {
        return patterns[currentPatternIndex];
    }

    void rotatePattern();


    void setHealth(int h);

    void setNowPosition(int row, int col) {
        nowRow = row;
        nowCol = col;
    }

    int getNowRow() const { return nowRow; }
    int getNowCol() const { return nowCol; }

    int getHealth() const{ return health; }

    void getDamage(int damage);

    bool TryMoveTowardPlayer(
        int playerRow,
        int playerCol,
        int gridStartRow, int gridEndRow,
        int gridStartCol, int gridEndCol,
        const std::vector<Enemy*>& allEnemies,
		int& outR, int& outC
    );

    void setObject(SpriteObject* obj) { objSprite = obj; }
    SpriteObject* getObject() { return objSprite; }

    TextObject* getHPText() { return hpText; }
    TextObject* getCorruptText() { return corruptText; }
    TextObject* getDebuffText() { return debuffText; }

    void showAttackText() {
        attackTextTimer = 1.0f;
    }
    void UpdateTextPosition();
    void Update(float dt);
    void SetWorldPosition(glm::vec3 pos);

    void addDelay(int turns);
    bool isDelayed() const;
    int getDelayTurns() const { return delayTurns; }
    void decrementDelay();

    void PlayAttackAnimation(glm::vec3 playerPos);

    void addCorruption(int stacks);
    int getCorruption() const { return corruptionStacks; }

    void addWeaken(int turns);
    int getWeakenTurns() const { return weakenTurns; }
    void decrementWeaken();
    int getAttackDamage() const;

    bool isPreparingAttack()const;
    void setPreparingAttack(bool value);

    bool getIsDead() const { return isDead; }
    void markDead() { isDead = true; }

	bool getIsMoving() const { return isMoving; }

    void StartMove(glm::vec3 targetWorld);
    int highlightIndex;
private:
    void RefreshDebuffText();

    EnemyType type;

    int maxHealth = 10;
    int health = maxHealth;
    int damage = 1;

    int nowRow = 0;
    int nowCol = 0;

    bool isMoving = false;
    glm::vec3 moveStart;
    glm::vec3 moveTarget;
    float moveTimer = 0.0f;
    float moveDuration = 1.25f;//move duration

    bool preparingAttack = false;
    bool isDead = false;

    bool isTakingDamage = false;
    float damageTimer = 0.0f;
    float damageDuration = 2.0f;

    bool isAttacking = false;
    float attackTimer = 0.0f;
	float attackDuration = 1.0f;//attack animation duration

    SpriteObject* objSprite = nullptr;
    std::vector<AttackPattern> patterns;
    int currentPatternIndex = 0;

    TextObject* hpText = nullptr;
    TextObject* corruptText = nullptr;
    TextObject* debuffText = nullptr;

    float attackTextTimer = 0.0f;

    int delayTurns = 0;
    int corruptionStacks = 0;
    int weakenTurns = 0;
};
