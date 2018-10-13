#pragma once

#include <weapon/SemiAutoBuilder.h>

class MagnumBuilder: public SemiAutoBuilder {
public:
	MagnumBuilder() :
			SemiAutoBuilder(0.7f) {
	}

	Weapon* build(edict_t* weap);
};
