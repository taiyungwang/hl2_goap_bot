#pragma once

#include <weapon/FullAutoGunBuilder.h>

class AR2Builder: public FullAutoGunBuilder {
public:
	AR2Builder() :
			FullAutoGunBuilder(0.4f, 0.92f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;

};
