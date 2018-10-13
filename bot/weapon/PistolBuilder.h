#pragma once

#include "SemiAutoBuilder.h"

class PistolBuilder: public SemiAutoBuilder {
public:
	PistolBuilder(float damage1) :
			SemiAutoBuilder(damage1) {
	}

	Weapon* build(edict_t* weap);

};
