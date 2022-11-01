#pragma once

#include <weapon/WeaponBuilder.h>
#include <string>
#include <unordered_map>
#include <memory>

class Weapon;
class Blackboard;
struct edict_t;


typedef bool (*WeaponFilter)(const Weapon*, Blackboard& blackboard, float dist);
typedef std::unordered_map<std::string, std::shared_ptr<WeaponBuilder>> WeaponBuilders;

class Arsenal {
public:
	Arsenal(const WeaponBuilders& builders) : builders(builders) {
		reset();
	}

	~Arsenal() {
		reset();
	}

	int getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const;

	void reset();

	void update(edict_t* self);

	int getCurrWeaponIdx() const {
		return currWeapIdx;
	}

	/**
	 * @return The current Weapon
	 */
	Weapon *getCurrWeapon() const {
		return getWeapon(currWeapIdx);
	}

	Weapon* getWeapon(int key) const {
		return weapons.find(key) == weapons.end() ? nullptr : weapons.at(key).get();
	}

	template<typename Func>
	void visit(Func visitor) const {
		for (auto const weapon: weapons) {
			if (visitor(weapon.first, weapon.second.get())) {
				break;
			}
		}
	}

	/**
	 * @return Weapon id if the arsenal contains a weapon matching current name,
	 * otherwise 0.
	 */
	int getWeaponIdByName(const char* name) const;

	int getDesiredWeaponIdx() const {
		return desiredWeapIdx;
	}

	void setDesiredWeaponIdx(int desiredWeapon) {
		this->desiredWeapIdx = desiredWeapon;
	}

private:
	int currWeapIdx, desiredWeapIdx = 0;

	std::unordered_map<int, std::shared_ptr<Weapon>> weapons;

	const WeaponBuilders& builders;
};
