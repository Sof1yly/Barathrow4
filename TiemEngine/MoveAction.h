#pragma once
#include "Action.h"

class MoveAction : public Action {
public:
	MoveAction() {
		setType(ActionType::Move);
		setMultiplier(1.0f);
	}

	bool isRetreat() const { return getActionCode() == "re"; }

	void do_action() {
		if (isRetreat())
			cout << "retreat " << getValue() << endl;
		else
			cout << "move " << getValue() << endl;
	}
};