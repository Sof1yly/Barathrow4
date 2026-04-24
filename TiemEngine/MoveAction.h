#pragma once
#include "Action.h"

enum class MoveSubType {
    Move,    // mov - move forward
    Retreat, // re  - move backward
    Unknown
};

class MoveAction : public Action {
private:
    MoveSubType subType = MoveSubType::Unknown;

public:
    MoveAction() {
        setType(ActionType::Move);
        setMultiplier(1.0f);
    }

    explicit MoveAction(MoveSubType sub) : MoveAction() {
        subType = sub;
    }

    MoveSubType getSubType() const { return subType; }
    void setSubType(MoveSubType s) { subType = s; }

    static MoveSubType codeToSubType(const std::string& code) {
        if (code == "mov") return MoveSubType::Move;
        if (code == "re")  return MoveSubType::Retreat;
        return MoveSubType::Unknown;
    }

    bool isRetreat() const { return subType == MoveSubType::Retreat; }

    // Executes this move: accumulates move or retreat steps
    void execute(CardPlayContext& ctx, CardPlayResult& result) override;

    void do_action() override {
        if (isRetreat())
            cout << "retreat " << getValue() << endl;
        else
            cout << "move " << getValue() << endl;
    }
};