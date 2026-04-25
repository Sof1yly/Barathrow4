#pragma once
#include "ImageObject.h"
#include "TextObject.h"
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
    int barrierCount = 0;
    int jumpCharges = 0;

	bool isDead = false;

	// Shield UI
	ImageObject* shieldBg = nullptr;
	ImageObject* shieldBar = nullptr;
	ImageObject* shieldMask = nullptr;
	TextObject* barrierText = nullptr;
	TextObject* jumpText = nullptr;
    SpriteObject* sprite = nullptr;
	void UpdateBarrierTextUI();
	void UpdateJumpTextUI();

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
    void AddBarrier(int amount = 1);
	bool ConsumeBarrier();
	void ExpireBarrier();
	bool HasBarrier() const;
    int GetBarrierCount() const;
    // =====================
    // Jump Charges
    // =====================
    void AddJumpCharges(int amount);
    bool ConsumeJumpCharge();
    void ResetJumpCharges();
    int GetJumpCharges() const;

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