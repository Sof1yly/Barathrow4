#pragma once

#include "Action.h"

class AttackAction :public Action {
	public:
		void do_action() {
			cout << "attack " << getValue();
		}
};