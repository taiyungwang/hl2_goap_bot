#pragma once

#include "Action.h"

class Armory;

class WeaponAction: public Action {
public:
	WeaponAction(Blackboard& blackboard);

protected:
	Armory& armory;
};
