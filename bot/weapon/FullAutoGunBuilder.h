#pragma once

#include "SemiAutoBuilder.h"

class FullAutoGunBuilder: public SemiAutoBuilder {
public:
virtual std::shared_ptr<Weapon> build(edict_t* weap) const;

protected:
	FullAutoGunBuilder(float damage1, float damage2) :
			SemiAutoBuilder(damage1) {
		this->damage2 = damage2;
	}

};
