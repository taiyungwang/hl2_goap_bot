#pragma once

class Weapon;
struct edict_t;

/**
 * A weapon builder.
 */
class WeaponBuilder {
public:
	virtual ~WeaponBuilder() {
	}

	virtual Weapon* build(edict_t* weap) = 0;

protected:
	float damage1, damage2;
};
