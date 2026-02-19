#pragma once
#include "Action.h"

class EnergyAction : public Action {
public:
	EnergyAction() {
		setType(ActionType::Energy);
		setMultiplier(1.0f);
	}

	void do_action() {
		cout << "energy " << getActionCode() << " " << getValue() << endl;
	}
};
