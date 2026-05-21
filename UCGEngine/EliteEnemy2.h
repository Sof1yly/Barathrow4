#pragma once
#include "EliteEnemy.h"
#include <vector>

// Elite type 2 — Pattern Caster
//   - Never moves (CanMove returns false)
//   - Each turn cycles through 4 attack patterns (1->2->3->4->1 ...)
//   - If the player is inside the current pattern -> use the pattern
//   - If the player is out of range -> cross attack centred on locked player pos
//
// Sprite sheet layout (4 rows x 7 cols, left-facing only — right is X-flipped):
//   Row 0 : Idle       (6 frames)
//   Row 1 : Attack     (6 frames)
//   Row 2 : Get Damage (6 frames)
//   Row 3 : Death      (6 frames)
class EliteEnemy2 : public EliteEnemy
{
public:
    EliteEnemy2();
    ~EliteEnemy2() override = default;

    void Update(float dt) override;

    bool CanMove() const override { return false; }

    void PlayIdleAnimation()   override;
    void PlayAttackAnimation(glm::vec3 playerPos) override;

    void AdvancePattern();
    int  getCurrentPhase() const { return currentPhase; }

    bool IsPlayerInPatternRange(int playerRow, int playerCol) const;

    // Returns per-phase damage: 10 / 20 / 30 / 40
    int getAttackDamage() const override;

    std::vector<std::pair<int, int>> GetCrossAttackTiles(
        int playerRow, int playerCol) const;

    std::vector<std::pair<int, int>> GetCurrentPatternTiles() const;

protected:
    void PlayDeathAnimation()  override;
    void PlayDamageAnimation() override;

private:
    int currentPhase = 0;   // 0-3

    void BuildPatterns();
};
