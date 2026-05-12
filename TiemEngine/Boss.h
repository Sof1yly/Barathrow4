#pragma once
#include "Enemy.h"
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

    bool OccupiesTile(int row, int col) const override;

    std::vector<std::pair<int, int>> GetOccupiedTiles() const;

private:

    int hitboxTopOffset = -2;
    int hitboxBottomOffset = 2;

    int hitboxLeftOffset = 0;
    int hitboxRightOffset = 1;
};