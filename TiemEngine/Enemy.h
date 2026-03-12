#pragma once
#include "ImageObject.h"
#include "AttackPattern.h"
#include "TextObject.h"

class Enemy {
public:
    Enemy();
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

    void setObject(ImageObject* obj) { objImg = obj; }
    ImageObject* getObject() { return objImg; }

    TextObject* getHPText() { return hpText; }
    TextObject* getCorruptText() { return corruptText; }

    void showAttackText() {
        attackTextTimer = 1.0f;
    }
    void UpdateTextPosition();
    void Update(float dt);

    void addDelay(int turns);
    bool isDelayed() const;
    int getDelayTurns() const { return delayTurns; }
    void decrementDelay();

    void addCorruption(int stacks);
    int getCorruption() const { return corruptionStacks; }
private:
    int maxHealth = 10;
    int health = maxHealth;
    int damage = 1;

    int nowRow = 0;
    int nowCol = 0;

    ImageObject* objImg = nullptr;
    std::vector<AttackPattern> patterns;
    int currentPatternIndex = 0;

    TextObject* hpText = nullptr;
    TextObject* corruptText = nullptr;

    float attackTextTimer = 0.0f;

    int delayTurns = 0;
    int corruptionStacks = 0;
};
