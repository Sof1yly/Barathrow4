#pragma once
#include "Action.h"

class MoveAction : public Action {
public:
	MoveAction() {
		setType(ActionType::Move);
		setMultiplier(1.0f);
	}

	void do_action() {
		cout << "move " << getValue() << endl;
	}
};