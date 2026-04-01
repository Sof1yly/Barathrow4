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

    void do_action() override {
        cout << "debuff [" << getActionCode() << "] " << getValue();
        if (getMultiplier() != 1.0f)
            cout << " x" << getMultiplier();
        cout << endl;
    }
};
