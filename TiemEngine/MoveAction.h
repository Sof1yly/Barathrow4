#pragma once
#include "Action.h"

class MoveAction :public Action{
	public:
		void do_action() {
			cout << "move " << getValue();
		}
};