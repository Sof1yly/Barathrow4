#pragma once
#include "Enemy.h"

// Battery — spawned when boss HP drops to 50%.
// While any Battery is alive, the player cannot deal damage to the Boss.
// The Battery itself does not move or attack.
//
// Sprite sheet: battery.png  (3 rows × 7 cols)
//   Row 0 : Idle       ( 1 frame  )
//   Row 1 : Get Damage ( 5 frames )
//   Row 2 : Death      ( 7 frames )
class Battery : public Enemy
{
public:
    Battery();

    void Update(float dt) override;

    // Battery never moves
    bool CanMove() const override { return false; }

    // Battery never attacks
    void TakeTurn() override {}

    // Completely overrides base so death animation plays before isDead becomes true
    void getDamage(int damage) override;

    // Uses battery-specific idle (1 frame) instead of the generic 2-frame base idle
    void PlayIdleAnimation() override;

private:
    bool isDeadAnimating = false;   // death anim is playing; isDead is set after it finishes

    void PlayBatteryIdle();
};
