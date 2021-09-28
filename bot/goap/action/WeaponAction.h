#pragma once

#include "Action.h"

class Arsenal;

class WeaponAction: public Action {
public:
	WeaponAction(Blackboard& blackboard);

protected:
	Arsenal& arsenal;
};
