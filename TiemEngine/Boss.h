#pragma once
#include "Enemy.h"

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
};