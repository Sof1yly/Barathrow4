#pragma once
#include "EliteEnemy.h"
#include <vector>
#include <utility>

// Elite type 3 — Lunge Striker
//   - Can move (moveRange = 4)
//   - Alternates between two attack patterns each turn it fires:
//       Phase 0: wide directional sweep (rotates toward player)
//       Phase 1: straight line charge — moves to the farthest attacked tile after
//                the attack; if the player occupies that tile, damage is doubled
//                and the enemy stays put.
//
// Sprite sheet layout (4 rows x 7 cols, left-facing only — right is X-flipped):
//   Row 0 : Idle       (6 frames)
//   Row 1 : Attack     (7 frames)
//   Row 2 : Get Damage (3 frames)
//   Row 3 : Death      (7 frames)
class EliteEnemy3 : public EliteEnemy
{
public:
    EliteEnemy3();
    ~EliteEnemy3() override = default;

    void Update(float dt) override;
    void PlayIdleAnimation()               override;
    void PlayAttackAnimation(glm::vec3 playerPos) override;

    void AdvancePattern();
    bool IsLineAttack() const { return patternPhase == 1; }

    // Called from Level before LockAttackPattern: picks sweep always, or random when aligned.
    void SelectPattern(int playerRow, int playerCol);

    // Returns world-space hit tiles for the current pattern rotated toward (playerRow, playerCol).
    std::vector<std::pair<int, int>> GetPatternTiles(int playerRow, int playerCol) const;

    // For pattern 1 (line attack): returns the farthest tile that is attacked.
    std::pair<int, int> GetLineEndCell(int playerRow, int playerCol) const;

protected:
    void PlayDeathAnimation()  override;
    void PlayDamageAnimation() override;

private:
    int patternPhase = 0;   // 0 = sweep, 1 = line charge

    void BuildPatterns();
};
