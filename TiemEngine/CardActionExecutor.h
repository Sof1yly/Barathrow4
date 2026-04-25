#pragma once

#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include "Card.h"
#include "AttackPattern.h"

// Orchestrates card play: validates, executes all actions, applies attack patterns.
// This replaces the ~350 lines of inline logic that was in Level::HandleMouse().
class CardActionExecutor
{
public:
    // Execute all actions on a card, returning the accumulated result.
    // Handles: energy validation, overclock, action dispatch, energy generation.
    static CardPlayResult ExecuteCard(Card* card, CardPlayContext& ctx);

    // Apply all queued attack patterns to the grid, dealing damage and
    // applying debuffs (delay, weaken, corruption) to hit enemies.
    static void ApplyAttackPatterns(CardPlayResult& result, CardPlayContext& ctx);

    // Compute the retreat direction (opposite of the given drop zone).
    static int GetRetreatDirection(int dropZone);

    // Rotate an attack pattern to match the player's facing direction.
    static AttackPattern OrientPattern(const AttackPattern& base, int dropZone);
};
