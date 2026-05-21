#include "BuffAction.h"
#include "CardPlayContext.h"
#include "CardPlayResult.h"
#include "Player.h"
#include <algorithm>
#include <iostream>

void BuffAction::execute(CardPlayContext& ctx, CardPlayResult& result)
{
    switch (subType)
    {
    case BuffSubType::Shield:
        ctx.player.AddShield(getValue());
        std::cout << "  ShieldBuff: +" << getValue() << std::endl;
        break;

    case BuffSubType::Barrier:
    {
        int addBarrier = std::max(1, getValue());
        ctx.player.AddBarrier(addBarrier);
        std::cout << "  BarrierBuff: +" << addBarrier << std::endl;
        break;
    }

    case BuffSubType::Jump:
    {
        int charges = std::max(1, getValue());
        ctx.player.AddJumpCharges(charges);
        std::cout << "  JumpBuff: +" << charges << " jump charge(s)" << std::endl;
        break;
    }

    default:
        std::cout << "  Buff [" << getActionCode() << "] " << getValue() << std::endl;
        break;
    }
}
