#pragma once

#include <vector>
#include <string>

class AttackAction;
class AttackPattern;

struct PendingAttackInfo {
    AttackAction* atk;
    const AttackPattern* pattern;
    int resolvedTotalDamage = 0;  // snapshotted in execute() before ResetOverclock can run
    int resolvedPerHitDamage = 0; // per-repeat damage, used for popup numbers
};

// A single enemy-hit event: grid position + damage dealt
struct HitInfo {
    int row;
    int col;
    int damage;
    int repeatIndex = 0;
};

// Accumulated results from executing all actions on a card
struct CardPlayResult {
    int moveSteps = 0;
    int retreatSteps = 0;

    std::vector<PendingAttackInfo> pendingAttacks;
    int pendingDelayTurns = 0;
    int pendingCorruptionStacks = 0;
    int pendingWeakenTurns = 0;

    // "all enemy" variants — applied to every living enemy regardless of attack pattern
    int pendingWeakenAllTurns    = 0;
    int pendingDelayAllTurns     = 0;
    int pendingCorruptAllStacks  = 0;

    int energyGenerated = 0;

    // Combo: card names to add to hand after this card is played
    std::vector<std::string> comboCardNames;

    bool isFastCard = false;
    bool isLagCard = false;
    bool isDeleteAfterUse = false;

    // Populated by ApplyAttackPatterns – used to spawn damage popups
    std::vector<HitInfo> hits;

    bool hasAttack() const { return !pendingAttacks.empty(); }
};
