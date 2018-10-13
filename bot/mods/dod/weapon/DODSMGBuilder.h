#pragma once

#include <weapon/FullAutoGunBuilder.h>

class DODSMGBuilder: public FullAutoGunBuilder {
public:
	DODSMGBuilder() :
			FullAutoGunBuilder(0.3f, 0.6f) {
	}

	Weapon* build(edict_t* weap);

};
