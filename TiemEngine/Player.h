#pragma once
#include "ImageObject.h"
#include <algorithm>
#include <iostream>

class Player {
private:
	int hp = 5;
	int maxHp = 10;
	int shield = 0;
	int maxShield = 0;

	// Shield UI
	ImageObject* shieldBg = nullptr;
	ImageObject* shieldBar = nullptr;
	ImageObject* shieldMask = nullptr;

public:
	int getHp() const;
	void setHp(int value);
	int getMaxHp() const;
	void setMaxHp(int value);

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
};