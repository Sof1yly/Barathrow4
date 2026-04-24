#pragma once
#include "Action.h"

enum class EnergySubType {
    EnergyCard,  // enr - energy card that's always in the player's hand
    Generate,    // gen - generate energy cards (gen:2 = generate 2 energy cards)
    Consume,     // con - requires energy cards in hand to play (con:2 = need 2 energy cards)
    Unknown
};

class EnergyAction : public Action {
private:
    EnergySubType subType = EnergySubType::Unknown;

public:
    EnergyAction() {
        setType(ActionType::Energy);
        setMultiplier(1.0f);
    }

    explicit EnergyAction(EnergySubType sub) : EnergyAction() {
        subType = sub;
    }

    EnergySubType getSubType() const { return subType; }
    void setSubType(EnergySubType s) { subType = s; }

    static EnergySubType codeToSubType(const std::string& code) {
        if (code == "enr") return EnergySubType::EnergyCard;
        if (code == "gen") return EnergySubType::Generate;
        if (code == "con") return EnergySubType::Consume;
        return EnergySubType::Unknown;
    }

    // Executes this energy action: generates energy cards
    void execute(CardPlayContext& ctx, CardPlayResult& result) override;

    Action* clone() const override {
        auto* c = new EnergyAction(subType);
        c->setValue(getValue());
        c->setBaseValue(getBaseValue());
        c->setMultiplier(getMultiplier());
        c->setRepeatCount(getRepeatCount());
        c->setRotation(getRotation());
        c->setActionCode(getActionCode());
        return c;
    }

    void do_action() override {
        cout << "energy [" << getActionCode() << "] " << getValue() << endl;
    }
};
