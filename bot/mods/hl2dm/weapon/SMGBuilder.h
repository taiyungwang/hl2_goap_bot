#pragma once

#include <weapon/FullAutoGunBuilder.h>

class SMGBuilder: public FullAutoGunBuilder {
public:
	SMGBuilder() :
			FullAutoGunBuilder(0.3f, 0.9f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
