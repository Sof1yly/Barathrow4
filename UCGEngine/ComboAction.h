#pragma once
#include "Action.h"
#include <string>

// ComboAction: when this card is played, add a copy of the target card to hand.
// Example: "combo:Blade Throw" on Fans of Blade adds Blade Throw to the player's hand.
class ComboAction : public Action {
private:
    std::string targetCardName;

public:
    ComboAction() {
        setType(ActionType::Combo);
        setMultiplier(1.0f);
    }

    explicit ComboAction(const std::string& cardName) : ComboAction() {
        targetCardName = cardName;
    }

    const std::string& getTargetCardName() const { return targetCardName; }
    void setTargetCardName(const std::string& name) { targetCardName = name; }

    // Queues the combo card name into the result for later generation
    void execute(CardPlayContext& ctx, CardPlayResult& result) override;

    Action* clone() const override {
        auto* c = new ComboAction(targetCardName);
        c->setValue(getValue());
        c->setBaseValue(getBaseValue());
        c->setMultiplier(getMultiplier());
        c->setRepeatCount(getRepeatCount());
        c->setRotation(getRotation());
        c->setActionCode(getActionCode());
        return c;
    }

    void do_action() override {
        cout << "combo [" << targetCardName << "]" << endl;
    }
};
