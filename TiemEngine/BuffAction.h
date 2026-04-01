#pragma once
#include "Action.h"

enum class BuffSubType {
    Overclock,  // oc  - boost atk value
    Shield,     // sh  - grant shield
    Barrier,    // ba  - grant barrier
    Heal,       // he  - restore HP
    Fast,       // fas - this action executes without consuming a turn
    Persist, // per - this card remain on hand until player uses it
	PreLoad, //pre - this card will be in hand in the first turn
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
        return BuffSubType::Unknown;
    }

    void do_action() override {
        cout << "buff [" << getActionCode() << "] " << getValue() << endl;
    }
};
