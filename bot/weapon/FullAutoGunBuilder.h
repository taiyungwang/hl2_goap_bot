#pragma once

#include "SemiAutoBuilder.h"

class FullAutoGunBuilder: public SemiAutoBuilder {
protected:
	FullAutoGunBuilder(float damage1, float damage2) :
			SemiAutoBuilder(damage1) {
		this->damage2 = damage2;
	}

	Weapon* build(edict_t* weap);

};
