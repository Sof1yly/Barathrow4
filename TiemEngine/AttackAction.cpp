#include "AttackAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include "Player.h"
#include "CardSystem.h"
#include <iostream>

void AttackAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    int resolvedAttack = resolveDamage(ctx.player.getShield());

    std::cout << "  AttackAction: " << resolvedAttack;
    if (getRepeatCount() > 1) {
        std::cout << " x" << getRepeatCount();
    }
    if (subType == AttackSubType::ShieldScaled) {
        std::cout << " (shield-scaled)";
    }
    std::cout << std::endl;

    // Look up the attack pattern linked to this action
    const AttackPattern* basePat = ctx.cardSystem.GetDataLoader().getPatternForAction(this);
    if (!basePat) {
        std::cout << "    (no attack pattern linked to this action)\n";
    }

    // Queue this attack for pattern application later
    result.pendingAttacks.push_back({ this, basePat });
}
