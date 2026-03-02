#pragma once
#include "Action.h"

enum class BuffSubType {
    Overclock,  // oc  - boost atk value
    Shield,     // sh  - grant shield
    Barrier,    // ba  - grant barrier
    Guard,      // ge  - guard effect
    Heal,       // he  - restore HP
    Fast,       // fas - this action executes without consuming a turn
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
        if (code == "ba")  return BuffSubType::Barrier;
        if (code == "ge")  return BuffSubType::Guard;
        if (code == "he")  return BuffSubType::Heal;
        if (code == "fas") return BuffSubType::Fast;
        return BuffSubType::Unknown;
    }

    void do_action() override {
        cout << "buff [" << getActionCode() << "] " << getValue() << endl;
    }
};
