#pragma once

#include <weapon/GrenadeBuilder.h>
#include <set>
#include <string>

class DODLiveGrenadeBuilder: public GrenadeBuilder {
public:
	const static std::set<std::string> NAMES;

	DODLiveGrenadeBuilder() :
			GrenadeBuilder(600.0f) {
	}
};
