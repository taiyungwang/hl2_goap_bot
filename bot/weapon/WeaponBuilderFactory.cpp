#include "WeaponBuilderFactory.h"

#include "WeaponBuilder.h"

WeaponBuilderFactory::~WeaponBuilderFactory() {
	builders.Purge();
}

WeaponBuilder* WeaponBuilderFactory::getInstance(const char* name) const {
	auto i = builders.Find(CUtlString(name));
	return builders.IsValidIndex(i) ? builders[i] : nullptr;
}
