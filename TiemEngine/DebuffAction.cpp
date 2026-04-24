#include "DebuffAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include <iostream>

void DebuffAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    switch (subType)
    {
    case DebuffSubType::Delay:
        result.pendingDelayTurns += getValue();
        std::cout << "  DelayAction: " << getValue() << std::endl;
        break;

    case DebuffSubType::Weaken:
        result.pendingWeakenTurns += getValue();
        std::cout << "  WeakenAction: " << getValue() << std::endl;
        break;

    case DebuffSubType::Corrupt:
        result.pendingCorruptionStacks += getValue();
        std::cout << "  CorruptAction: " << getValue() << std::endl;
        break;

    default:
        std::cout << "  Debuff [" << getActionCode() << "] " << getValue() << std::endl;
        break;
    }
}
