#pragma once
#include "Action.h"

class BuffAction : public Action {
public:
	BuffAction() {
		setType(ActionType::Buff);
		setMultiplier(1.0f);
	}

	void do_action() {
		cout << "buff " << getActionCode() << " " << getValue() << endl;
	}
};
