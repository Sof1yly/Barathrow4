#pragma once
#include "Action.h"

enum class DebuffSubType {
    Weaken,  // wk - reduce enemy attack
    Delay,   // dl - delay enemy turn or action
    Lag, // lag - skip player next turn
    Corrupt, // cor - apply corruption, enemy takes extra damage
    Unknown
};

class DebuffAction : public Action {
private:
    DebuffSubType subType = DebuffSubType::Unknown;

public:
    DebuffAction() {
        setType(ActionType::Debuff);
        setMultiplier(1.0f);
    }

    explicit DebuffAction(DebuffSubType sub) : DebuffAction() {
        subType = sub;
    }

    DebuffSubType getSubType() const { return subType; }
    void setSubType(DebuffSubType s) { subType = s; }

    static DebuffSubType codeToSubType(const std::string& code) {
        if (code == "wk" || code == "wek") return DebuffSubType::Weaken;
        if (code == "dl" || code == "de") return DebuffSubType::Delay;
        if (code == "lag") return DebuffSubType::Lag;
        if (code == "cor") return DebuffSubType::Corrupt;
        return DebuffSubType::Unknown;
    }

    // Executes this debuff: queues delay, weaken, or corruption for enemies
    void execute(CardPlayContext& ctx, CardPlayResult& result) override;

    Action* clone() const override {
        auto* c = new DebuffAction(subType);
        c->setValue(getValue());
        c->setBaseValue(getBaseValue());
        c->setMultiplier(getMultiplier());
        c->setRepeatCount(getRepeatCount());
        c->setRotation(getRotation());
        c->setActionCode(getActionCode());
        return c;
    }

    void do_action() override {
        cout << "debuff [" << getActionCode() << "] " << getValue();
        if (getMultiplier() != 1.0f)
            cout << " x" << getMultiplier();
        cout << endl;
    }
};
