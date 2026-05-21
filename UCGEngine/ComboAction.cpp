#include "ComboAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include <iostream>

void ComboAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    if (!targetCardName.empty())
    {
        result.comboCardNames.push_back(targetCardName);
        std::cout << "  ComboAction: will add \"" << targetCardName << "\" to hand" << std::endl;
    }
}
