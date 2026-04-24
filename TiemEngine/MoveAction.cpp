#include "MoveAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include <iostream>

void MoveAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    if (isRetreat())
    {
        result.retreatSteps += getValue();
        std::cout << "  RetreatAction: " << getValue() << std::endl;
    }
    else
    {
        result.moveSteps += getValue();
        std::cout << "  MoveAction: " << getValue() << std::endl;
    }
}
