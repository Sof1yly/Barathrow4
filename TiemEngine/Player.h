#pragma once
#include "ImageObject.h"
#include "SpriteObject.h"
#include <vector>
#include <algorithm>
#include <iostream>

class Player {
private:
    int hp = 5;
    int maxHp = 10;

    int shield = 0;
    int maxShield = 0;

    // Sprite
    SpriteObject* sprite = nullptr;

    // Shield UI
    ImageObject* shieldBg = nullptr;
    ImageObject* shieldBar = nullptr;
    ImageObject* shieldMask = nullptr;

public:
    // =====================
    // HP
    // =====================
    int getHp() const;
    void setHp(int value);
    int getMaxHp() const;
    void setMaxHp(int value);

    // =====================
    // Shield
    // =====================
    int getShield() const;
    int getMaxShield() const;

    void AddShield(int amount);
    void ResetShield();
    int AbsorbDamage(int damage);

    void InitShieldUI(std::vector<DrawableObject*>& objectsList);
    void UpdateShieldBar();

    ImageObject* GetShieldBg() const { return shieldBg; }
    ImageObject* GetShieldBar() const { return shieldBar; }
    ImageObject* GetShieldMask() const { return shieldMask; }

    // =====================
    // Sprite
    // =====================
    void InitSprite(std::vector<DrawableObject*>& objectsList, glm::vec3 startPos);
    SpriteObject* GetSprite();

    void SetPosition(glm::vec3 pos);
    glm::vec3 GetPosition() const;

    // =====================
    // Animation
    // dir: 0=DOWN, 1=LEFT, 2=UP, 3=RIGHT
    // =====================
    void SetPlayerIdle(int dir);
    void SetPlayerWalk(int dir);
    void SetPlayerAttack(int dir);
    void SetPlayerGetDamage(int dir);
    void SetPlayerDie(int dir);
};