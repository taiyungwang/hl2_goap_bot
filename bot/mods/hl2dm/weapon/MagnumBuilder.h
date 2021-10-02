#pragma once

#include <weapon/SemiAutoBuilder.h>

class MagnumBuilder: public SemiAutoBuilder {
public:
	MagnumBuilder() :
			SemiAutoBuilder(0.7f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
