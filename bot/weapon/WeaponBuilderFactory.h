#pragma once

#include <utlmap.h>
#include <utlstring.h>

class WeaponBuilder;

/**
 * Weapon builder factory.
 */
class WeaponBuilderFactory {
public:
	WeaponBuilderFactory() {
		builders.SetLessFunc([] (const CUtlString& s1, const CUtlString& s2) {
			return StringFuncs<char>::Compare(s1.Get(), s2.Get()) < 0;
		});
	}

	~WeaponBuilderFactory();

	void addInstance(const char* name, WeaponBuilder* builder) {
		builders.Insert(CUtlString(name), builder);
	}

	WeaponBuilder* getInstance(const char* name) const;

private:
	CUtlMap<CUtlString, WeaponBuilder*> builders;
};
