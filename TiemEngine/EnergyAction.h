#pragma once
#include "Action.h"

enum class EnergySubType {
    Gain,      // en  - gain energy points
    Convert,   // co  - convert resource to energy
    GainCard,  // gc  - gain energy and draw a card
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
        if (code == "en") return EnergySubType::Gain;
        if (code == "co") return EnergySubType::Convert;
        if (code == "gc") return EnergySubType::GainCard;
        return EnergySubType::Unknown;
    }

    void do_action() override {
        cout << "energy [" << getActionCode() << "] " << getValue() << endl;
    }
};
