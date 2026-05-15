#pragma once
#include "EliteEnemy.h"
#include <vector>

// Elite type 2 — Pattern Caster
//   - Never moves (CanMove returns false)
//   - Each turn cycles through 4 attack patterns (1 → 2 → 3 → 4 → 1 ...)
//   - If the player is inside the current pattern's tiles → use the pattern
//   - If the player is out of range → fire a cross attack:
//       player's tile + the 4 orthogonal neighbours (always hits)
class EliteEnemy2 : public EliteEnemy
{
public:
    EliteEnemy2();
    ~EliteEnemy2() override = default;

    void Update(float dt) override;

    bool CanMove() const override { return false; }

    // Advance to the next phase (call once per turn after attacking).
    void AdvancePattern();

    int getCurrentPhase() const { return currentPhase; }

    // Returns true when the player's tile falls inside the current pattern.
    bool IsPlayerInPatternRange(int playerRow, int playerCol) const;

    // Cross of 5 tiles centred on the player — used when out of pattern range.
    std::vector<std::pair<int, int>> GetCrossAttackTiles(
        int playerRow,
        int playerCol) const;

    // World-space tile list for the current pattern (relative to this elite's pos).
    std::vector<std::pair<int, int>> GetCurrentPatternTiles() const;

private:
    int currentPhase = 0; // 0-3 → patterns 1-4

    void BuildPatterns();
};
