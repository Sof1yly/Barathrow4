#pragma once

#include <vector>
#include <string>

class AttackAction;
class AttackPattern;

struct PendingAttackInfo {
    AttackAction* atk;
    const AttackPattern* pattern;
};

// Accumulated results from executing all actions on a card
struct CardPlayResult {
    int moveSteps = 0;
    int retreatSteps = 0;

    std::vector<PendingAttackInfo> pendingAttacks;
    int pendingDelayTurns = 0;
    int pendingCorruptionStacks = 0;
    int pendingWeakenTurns = 0;

    int energyGenerated = 0;

    // Combo: card names to add to hand after this card is played
    std::vector<std::string> comboCardNames;

    bool isFastCard = false;
    bool isLagCard = false;
    bool isDeleteAfterUse = false;

    bool hasAttack() const { return !pendingAttacks.empty(); }
};
