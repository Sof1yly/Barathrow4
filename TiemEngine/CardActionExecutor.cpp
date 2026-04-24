#include "CardActionExecutor.h"
#include "CardSystem.h"
#include "Player.h"
#include "Enemy.h"
#include "AttackAction.h"
#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------
// ExecuteCard  -  run every Action on the card, collect results
// ---------------------------------------------------------------
CardPlayResult CardActionExecutor::ExecuteCard(Card* card, CardPlayContext& ctx)
{
    CardPlayResult result;

    // Copy card-level flags into the result so Level can read them
    result.isFastCard = card->getIsFast();
    result.isLagCard = card->getIsLag();
    result.isDeleteAfterUse = card->getIsDeleteAfterUse();

    // Apply overclock bonus before actions run
    if (card->getOverclockValue() > 0)
    {
        ctx.cardSystem.ApplyOverclock(card->getOverclockValue());
    }

    // Consume energy cards if required (already validated by caller)
    int conReq = card->getConsumeRequirement();
    if (conReq > 0)
    {
        ctx.cardSystem.ConsumeEnergyCards(conReq, ctx.objectsList);
    }

    // Dispatch every action - each subclass handles itself
    const auto& acts = card->getActions();
    for (Action* a : acts)
    {
        a->execute(ctx, result);
    }

    // Generate energy cards if any Generate actions accumulated
    if (result.energyGenerated > 0)
    {
        std::cout << "[Energy] Card generates " << result.energyGenerated
                  << " energy card(s)." << std::endl;
        ctx.cardSystem.GenerateEnergyCards(result.energyGenerated, ctx.objectsList);
    }

    // Generate combo cards (add target cards to hand)
    for (const std::string& comboName : result.comboCardNames)
    {
        ctx.cardSystem.GenerateComboCard(comboName, ctx.objectsList);
    }

    // Reset overclock after an attack card is played
    if (result.hasAttack())
    {
        ctx.cardSystem.ResetOverclock();
    }

    return result;
}

// ---------------------------------------------------------------
// ApplyAttackPatterns  -  resolve damage on the grid
// ---------------------------------------------------------------
void CardActionExecutor::ApplyAttackPatterns(const CardPlayResult& result, CardPlayContext& ctx)
{
    bool corruptionApplied = false;

    for (const PendingAttackInfo& pa : result.pendingAttacks)
    {
        AttackAction* atk = pa.atk;
        const AttackPattern* basePat = pa.pattern;
        int attackDamage = atk->resolveTotalDamage(ctx.player.getShield());

        if (!basePat) {
            continue;
        }

        // Rotate pattern to match the drop zone direction
        AttackPattern oriented = OrientPattern(*basePat, ctx.dropZone);

        auto cells = oriented.applyTo(ctx.playerRow, ctx.playerCol);
        std::cout << "    Applying attack pattern from ("
                  << ctx.playerRow << ", " << ctx.playerCol << ")\n";

        for (auto& cell : cells)
        {
            int gx = cell.first.x;
            int gy = cell.first.y;

            // Skip out-of-bounds cells
            if (gx < ctx.gridStartRow || gx >= ctx.gridEndRow ||
                gy < ctx.gridStartCol || gy >= ctx.gridEndCol)
            {
                std::cout << "      Skip out-of-bounds cell (" << gx << ", " << gy << ")\n";
                continue;
            }

            std::cout << "      Attack cell (" << gx << ", " << gy << ")\n";

            for (auto* e : ctx.enemies)
            {
                if (!e || e->getIsDead()) continue;
                if (e->getNowRow() == gx && e->getNowCol() == gy)
                {
                    e->getDamage(attackDamage);
                    std::cout << "        HIT enemy! HP: " << e->getHealth() << std::endl;

                    if (result.pendingDelayTurns > 0)
                    {
                        e->addDelay(result.pendingDelayTurns);
                    }

                    if (result.pendingWeakenTurns > 0)
                    {
                        e->addWeaken(result.pendingWeakenTurns);
                    }

                    // Corruption applies only to the first enemy hit
                    if (!corruptionApplied && result.pendingCorruptionStacks > 0)
                    {
                        e->addCorruption(result.pendingCorruptionStacks);
                        corruptionApplied = true;
                    }

                    if (e->getHealth() <= 0)
                    {
                        std::cout << "        Enemy died!\n";
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------
// GetRetreatDirection  -  opposite of the given drop zone
// ---------------------------------------------------------------
int CardActionExecutor::GetRetreatDirection(int dropZone)
{
    switch (dropZone)
    {
    case 0: return 3; // LEFT  -> RIGHT
    case 1: return 2; // UP    -> DOWN
    case 2: return 1; // DOWN  -> UP
    case 3: return 0; // RIGHT -> LEFT
    default: return dropZone;
    }
}

// ---------------------------------------------------------------
// OrientPattern  -  rotate pattern to match facing direction
// ---------------------------------------------------------------
AttackPattern CardActionExecutor::OrientPattern(const AttackPattern& base, int dropZone)
{
    // dropZone: 0=LEFT, 1=UP, 2=DOWN, 3=RIGHT
    // Base pattern faces DOWN (rotateTimes = 0)
    int rotateTimes = 0;

    switch (dropZone)
    {
    case 0: rotateTimes = 3; break; // LEFT
    case 1: rotateTimes = 2; break; // UP
    case 2: rotateTimes = 0; break; // DOWN
    case 3: rotateTimes = 1; break; // RIGHT
    }

    rotateTimes = (rotateTimes + 3) % 4;

    AttackPattern oriented = base;
    for (int i = 0; i < rotateTimes; i++)
    {
        oriented = oriented.rotated90CW();
    }

    return oriented;
}
