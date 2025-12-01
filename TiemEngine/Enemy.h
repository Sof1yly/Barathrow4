#pragma once
#include "ImageObject.h"
#include "AttackPattern.h"

class Enemy {
public:
    Enemy();

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

private:
    int maxHealth = 3;
    int health = maxHealth;
    int damage = 1;

    int nowRow = 0;
    int nowCol = 0;

    ImageObject* objImg = nullptr;
    std::vector<AttackPattern> patterns;
    int currentPatternIndex = 0;
};
