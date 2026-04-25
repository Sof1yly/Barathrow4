#pragma once
#include "Action.h"

enum class BuffSubType {
    Overclock,  // oc  - boost atk value
    Shield,     // sh  - grant shield
    Barrier,    // ba  - grant barrier
    Heal,       // he  - restore HP
    Fast,       // fas - this action executes without consuming a turn
    Persist,    // per - this card remain on hand until player uses it
    PreLoad,    // pre - this card will be in hand in the first turn
    Jump,       // jmp - grant N jump charges (pass through enemies)
    Unknown
};

class BuffAction : public Action {
private:
    BuffSubType subType = BuffSubType::Unknown;

public:
    BuffAction() {
        setType(ActionType::Buff);
        setMultiplier(1.0f);
    }

    explicit BuffAction(BuffSubType sub) : BuffAction() {
        subType = sub;
    }

    BuffSubType getSubType() const { return subType; }
    void setSubType(BuffSubType s) { subType = s; }

    static BuffSubType codeToSubType(const std::string& code) {
        if (code == "oc")  return BuffSubType::Overclock;
        if (code == "sh" || code == "shi")  return BuffSubType::Shield;
		if (code == "per") return BuffSubType::Persist;
        if (code == "ba" || code == "bar")  return BuffSubType::Barrier;
        if (code == "he")  return BuffSubType::Heal;
        if (code == "fas") return BuffSubType::Fast;
		if (code == "pre") return BuffSubType::PreLoad;
        if (code == "jmp") return BuffSubType::Jump;
        return BuffSubType::Unknown;
    }

    // Executes this buff: applies shield, barrier, etc. to the player
    void execute(CardPlayContext& ctx, CardPlayResult& result) override;

    Action* clone() const override {
        auto* c = new BuffAction(subType);
        c->setValue(getValue());
        c->setBaseValue(getBaseValue());
        c->setMultiplier(getMultiplier());
        c->setRepeatCount(getRepeatCount());
        c->setRotation(getRotation());
        c->setActionCode(getActionCode());
        return c;
    }

    void do_action() override {
        cout << "buff [" << getActionCode() << "] " << getValue() << endl;
    }
};
