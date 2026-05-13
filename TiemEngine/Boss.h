#pragma once
#include "Enemy.h"
#include "AttackPattern.h"
#include <vector>

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

private:
    int hitboxTopOffset = -2;
    int hitboxBottomOffset = 2;
    int hitboxLeftOffset = 0;
    int hitboxRightOffset = 1;
};