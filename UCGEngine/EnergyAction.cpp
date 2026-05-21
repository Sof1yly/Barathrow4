#include "EnergyAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include <iostream>

void EnergyAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    switch (subType)
    {
    case EnergySubType::Generate:
        result.energyGenerated += getValue();
        std::cout << "  GenerateEnergy: " << getValue() << std::endl;
        break;

    case EnergySubType::Consume:
        // Consume is handled as a pre-check before execute, not during
        std::cout << "  ConsumeEnergy: " << getValue() << " (already validated)" << std::endl;
        break;

    case EnergySubType::EnergyCard:
        // Energy cards themselves don't do anything when played
        std::cout << "  EnergyCard (no-op)" << std::endl;
        break;

    default:
        std::cout << "  Energy [" << getActionCode() << "] " << getValue() << std::endl;
        break;
    }
}
