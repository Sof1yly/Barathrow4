#pragma once
#include "Enemy.h"
#include "AttackPattern.h"
#include <vector>
#include <iostream>

class Boss : public Enemy
{
public:
    Boss();
    void Update(float dt) override;
    bool CanMove() const override
    {
        return false;
    }
    void TakeTurn() override;
    std::vector<std::pair<int, int>> GetOccupiedTiles() const override;
    bool OccupiesTile(int row, int col) const override;
    bool IsHitBy(const std::vector<std::pair<IVec2, int>>& cells);
    AttackPattern GetRotatedPatternTowardPlayer(int playerRow, int playerCol) const override;

    void PlayAttackAnimation(glm::vec3 playerPos) override;
    void getDamage(int damage) override;
    void setPreparingAttack(bool value) override;

    void setPlayerPosition(int row, int col)
    {
        playerRow = row;
        playerCol = col;
		std::cout << "Player position set to (" << playerRow << ", " << playerCol << ")\n";
	}

    int TryGetSummon();
private:
    int hitboxTopOffset = -2;
    int hitboxBottomOffset = 2;
    int hitboxLeftOffset = 0;
    int hitboxRightOffset = 1;

	int playerCol = 0;
	int playerRow = 0;

    void RollAttackPattern();

    int attackPatternChoice = 1;

    bool summoned66 = false;
    bool summoned33 = false;
};