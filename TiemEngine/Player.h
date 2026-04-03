#pragma once
#include "ImageObject.h"
#include "TextObject.h"
#include <algorithm>
#include <iostream>

class Player {
private:
	int hp = 5;
	int maxHp = 10;
	int shield = 0;
	int maxShield = 0;
  int barrierCount = 0;

	// Shield UI
	ImageObject* shieldBg = nullptr;
	ImageObject* shieldBar = nullptr;
	ImageObject* shieldMask = nullptr;
	TextObject* barrierText = nullptr;

	void UpdateBarrierTextUI();

public:
	int getHp() const;
	void setHp(int value);
	int getMaxHp() const;
	void setMaxHp(int value);

	int getShield() const;
	int getMaxShield() const;

	void AddShield(int amount);
   void AddBarrier(int amount = 1);
	bool ConsumeBarrier();
	void ExpireBarrier();
	bool HasBarrier() const;
   int GetBarrierCount() const;
	void ResetShield();
	int AbsorbDamage(int damage);

	void InitShieldUI(std::vector<DrawableObject*>& objectsList);
	void UpdateShieldBar();

	ImageObject* GetShieldBg() const { return shieldBg; }
	ImageObject* GetShieldBar() const { return shieldBar; }
	ImageObject* GetShieldMask() const { return shieldMask; }
};