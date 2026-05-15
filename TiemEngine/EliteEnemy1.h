#pragma once
#include "EliteEnemy.h"
#include <functional>
#include <vector>

// Elite type 1 — Row Striker
//   - Moves only vertically (up / down) toward the player's row
//   - On attack: hits every tile in its current row
//   - If the row attack lands on another EliteEnemy1, that elite is healed
//     instead of damaged (caller checks via ShouldHealInstead)
//   - Spawns at the horizontal edge (TODO: set in Level spawner)
class EliteEnemy1 : public EliteEnemy
{
public:
    EliteEnemy1();
    ~EliteEnemy1() override = default;

    void Update(float dt) override;

    // Returns every tile in this elite's row across the grid column range.
    // The Level uses this list to apply the row attack.
    std::vector<std::pair<int, int>> GetRowAttackTiles(
        int gridStartCol,
        int gridEndCol) const;

    // Returns true when 'other' is also an EliteEnemy1 — the Level should
    // heal 'other' by GetHealAmount() instead of dealing damage.
    bool ShouldHealInstead(const Enemy* other) const;

    int GetHealAmount() const { return healAmount; }

    // Vertical-only BFS step toward the player's row.
    // Replaces TryMoveTowardPlayer for this elite.
    bool TryMoveVertical(
        int playerRow,
        int gridStartRow, int gridEndRow,
        const std::vector<Enemy*>& allEnemies,
        const std::function<bool(int, int)>& isWalkable,
        int& outR, int& outC);

private:
    int healAmount = 5;     // HP restored to friendly Elite1 hit by row attack
};
