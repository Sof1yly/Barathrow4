#pragma once

#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include "Card.h"
#include "AttackPattern.h"


class CardActionExecutor
{
public:

    // Handles: energy validation, overclock, action dispatch, energy generation.
    static CardPlayResult ExecuteCard(Card* card, CardPlayContext& ctx);


    // applying debuffs (delay, weaken, corruption) to hit enemies.
    static void ApplyAttackPatterns(CardPlayResult& result, CardPlayContext& ctx);

    // Compute the retreat direction 
    static int GetRetreatDirection(int dropZone);

    // Rotate an attack pattern to match the player's facing direction.
    static AttackPattern OrientPattern(const AttackPattern& base, int dropZone);
};
