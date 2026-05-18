#pragma once
#include "Enemy.h"

class EliteEnemy : public Enemy
{
public:
    enum class EliteType { Row, Pattern };

    EliteEnemy(EliteType eliteType);
    virtual ~EliteEnemy() = default;

    EliteType getEliteType() const { return eliteType; }

    // Call every frame from Level so the sprite flips toward the player.
    void UpdateFacing(glm::vec3 playerWorldPos);

    void getDamage(int damage) override;

protected:
    EliteType eliteType;

    bool facingRight    = false;
    bool isDeadAnimating = false;
    float spriteSize    = 150.0f;   // absolute X size; negated for right-facing flip

    // Derived classes define the exact rows/frames for their sprite sheet.
    virtual void PlayDeathAnimation()   = 0;
    virtual void PlayDamageAnimation()  = 0;
};
