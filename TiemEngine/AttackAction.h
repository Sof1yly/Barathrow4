#pragma once

#include "Action.h"

class AttackAction : public Action {
public:
	AttackAction() {
		setType(ActionType::Attack);
		setMultiplier(1.0f);
	}

	void do_action() {
		cout << "attack " << getValue() << endl;
	}
};