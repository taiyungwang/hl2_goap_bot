#include "WeaponBuilderFactory.h"

#include "WeaponBuilder.h"

WeaponBuilderFactory::~WeaponBuilderFactory() {
	builders.PurgeAndDeleteElements();
}

WeaponBuilder* WeaponBuilderFactory::getInstance(const char* name) const {
	auto i = builders.Find(CUtlString(name));
	return builders.IsValidIndex(i) ? builders[i] : nullptr;
}
