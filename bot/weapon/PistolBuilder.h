#pragma once

#include "SemiAutoBuilder.h"

class PistolBuilder: public SemiAutoBuilder {
public:
	PistolBuilder(float damage1) :
			SemiAutoBuilder(damage1) {
	}

	virtual std::shared_ptr<Weapon> build(edict_t* weap) const;
};
