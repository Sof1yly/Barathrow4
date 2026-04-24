#pragma once

#include "Action.h"

enum class AttackSubType {
	Flat,
	ShieldScaled
};

class AttackAction : public Action {
private:
	AttackSubType subType = AttackSubType::Flat;

public:
	AttackAction() {
		setType(ActionType::Attack);
		setMultiplier(1.0f);
	}

	AttackSubType getSubType() const {
		return subType;
	}

	void setSubType(AttackSubType s) {
		subType = s;
	}

 int resolveDamage(int currentShield) const {
		if (subType == AttackSubType::ShieldScaled) {
			return currentShield;
		}

		return getValue();
	}

	int resolveTotalDamage(int currentShield) const {
		return resolveDamage(currentShield) * getRepeatCount();
	}

	// Executes this attack: resolves damage and queues pattern for grid application
	void execute(CardPlayContext& ctx, CardPlayResult& result) override;

	void do_action() override {
        if (subType == AttackSubType::ShieldScaled) {
			cout << "attack [shield-scaled]" << endl;
		}
		else {
			cout << "attack " << getValue() << endl;
		}
	}
};