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
    int  getAttackDamage() const override;

    void setPlayerPosition(int row, int col)
    {
        playerRow = row;
        playerCol = col;
		std::cout << "Player position set to (" "Row = "<< playerRow << ", Col =" << playerCol << ")\n";
        if (playerRow % 2 == 0 && playerCol % 2 != 0) {
            even = true;
        }else if(playerRow % 2 != 0 && playerCol % 2 == 0){
            even = true;
        }
        else {
            even = false;
		}
		cout << "Player is on " << (even ? "even" : "odd") << " tile.\n";
		/* // For reference, the grid is indexed as follows:
            1  2  3  4  5  6  7  8  9
			10 11 12 13 14 15 16 17 18
			19 20 21 22 23 24 25 26 27
			28 29 30 31 32 33 34 35 36
			37 38 39 40 41 42 43 44 45
        */
	}

    int TryGetSummon();
    int getAttackPatternChoice() const { return attackPatternChoice; }

    // Plays the battery-summon animation (row 7) — called by Level when batteries spawn
    void PlayBatterySummonAnimation();

    // Returns true (exactly once) when boss HP drops to ≤50% and batteries haven't been spawned yet.
    bool ShouldSpawnBatteries();

private:
    bool batteriesSummoned = false; // ensures batteries spawn only once
    int hitboxTopOffset = -2;
    int hitboxBottomOffset = 2;
    int hitboxLeftOffset = 0;
    int hitboxRightOffset = 1;

    bool even;

	int playerCol = 0;
	int playerRow = 0;

    void RollAttackPattern();

    int attackPatternChoice = 1;

    bool lastWasCross = false;      // prevents cross attack from being picked twice in a row

    // Summon is a normal skill now (choice 9).
    // It re-enters the pool only after the boss uses ≥3 other skills (not guaranteed).
    int skillsSinceLastSummon = 0;
};