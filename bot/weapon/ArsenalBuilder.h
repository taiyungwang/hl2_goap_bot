#pragma once

#include "Arsenal.h"

class ArsenalBuilder {
public:
	virtual ~ArsenalBuilder() {}

	std::shared_ptr<Arsenal> build() const {
		return std::make_shared<Arsenal>(weaponBuilders);
	}

protected:
	WeaponBuilders weaponBuilders;

	ArsenalBuilder() {
	}
};
