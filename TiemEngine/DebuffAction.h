#pragma once
#include "Action.h"

class DebuffAction : public Action {
public:
	DebuffAction() {
		setType(ActionType::Debuff);
		setMultiplier(1.0f);
	}

	DebuffAction(float mult) {
		setType(ActionType::Debuff);
		setMultiplier(mult);
	}

	void do_action() {
		cout << "debuff " << getActionCode() << " " << getValue();
		if (getMultiplier() != 1.0f) {
			cout << " x" << getMultiplier();
		}
		cout << endl;
	}
};
