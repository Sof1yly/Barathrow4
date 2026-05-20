#pragma once
#include "EliteEnemy.h"
#include <functional>
#include <vector>

// Elite type 1 — Row Striker
//   - Moves only vertically (up / down) toward the player's column
//   - On attack: hits every tile in its current column (horizontal sweep)
//   - If the column attack lands on another EliteEnemy1, that elite is healed
//     instead of damaged (caller checks via ShouldHealInstead)
//
// Sprite sheet layout (8 rows x 12 cols, left-facing only — right is X-flipped):
//   Row 0 : Idle       (12 frames)
//   Row 1 : Walk       (12 frames)
//   Row 2 : Attack     ( 8 frames)
//   Row 3 : Get Damage ( 6 frames)
//   Row 4 : Death      (10 frames)
class EliteEnemy1 : public EliteEnemy
{
public:
    EliteEnemy1();
    ~EliteEnemy1() override = default;

    void Update(float dt) override;

    void PlayIdleAnimation()   override;
    void PlayAttackAnimation(glm::vec3 playerPos) override;

    // Returns every tile in this elite's column across the grid row range.
    std::vector<std::pair<int, int>> GetRowAttackTiles(
        int gridStartRow,
        int gridEndRow) const;

    bool ShouldHealInstead(const Enemy* other) const;
    int  GetHealAmount() const { return healAmount; }

    bool TryMoveTowardPlayer(
        int playerRow, int playerCol,
        int gridStartRow, int gridEndRow,
        int gridStartCol, int gridEndCol,
        const std::vector<Enemy*>& allEnemies,
        const std::function<bool(int, int)>& isWalkable,
        int& outR, int& outC) override;

protected:
    void PlayDeathAnimation()  override;
    void PlayDamageAnimation() override;

private:
    int healAmount = 10;

    int moveCooldown    = 0;  // turns remaining before this elite can move again
    int moveCooldownMax = 1;  // turns to wait after each move
};
