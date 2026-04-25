#include "DebuffAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include <iostream>

void DebuffAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    switch (subType)
    {
    case DebuffSubType::Delay:
        if (applyToAll)
        {
            result.pendingDelayAllTurns += getValue();
            std::cout << "  DelayAction (ALL): " << getValue() << std::endl;
        }
        else
        {
            result.pendingDelayTurns += getValue();
            std::cout << "  DelayAction: " << getValue() << std::endl;
        }
        break;

    case DebuffSubType::Weaken:
        if (applyToAll)
        {
            result.pendingWeakenAllTurns += getValue();
            std::cout << "  WeakenAction (ALL): " << getValue() << std::endl;
        }
        else
        {
            result.pendingWeakenTurns += getValue();
            std::cout << "  WeakenAction: " << getValue() << std::endl;
        }
        break;

    case DebuffSubType::Corrupt:
        if (applyToAll)
        {
            result.pendingCorruptAllStacks += getValue();
            std::cout << "  CorruptAction (ALL): " << getValue() << std::endl;
        }
        else
        {
            result.pendingCorruptionStacks += getValue();
            std::cout << "  CorruptAction: " << getValue() << std::endl;
        }
        break;

    default:
        std::cout << "  Debuff [" << getActionCode() << "] " << getValue() << std::endl;
        break;
    }
}
