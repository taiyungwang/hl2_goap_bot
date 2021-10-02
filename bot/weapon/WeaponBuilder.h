#pragma once

#include <memory>

class Weapon;
struct edict_t;

/**
 * A weapon builder.
 */
class WeaponBuilder {
public:
	virtual ~WeaponBuilder() {
	}

	virtual std::shared_ptr<Weapon> build(edict_t* weap) const = 0;

protected:
	float damage1, damage2;
};
