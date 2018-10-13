#pragma once

#include <weapon/FullAutoGunBuilder.h>

class SMGBuilder: public FullAutoGunBuilder {
public:
	SMGBuilder() :
			FullAutoGunBuilder(0.3f, 0.9f) {
	}

	Weapon* build(edict_t* weap);

};
