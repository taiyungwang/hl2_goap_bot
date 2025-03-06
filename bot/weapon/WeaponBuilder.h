#pragma once

#include <memory>
#include <unordered_map>
#include <string>

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

typedef std::unordered_map<std::string, std::shared_ptr<WeaponBuilder>> WeaponBuilders;

