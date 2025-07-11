#pragma once

class Buttons;
class Vector;
struct edict_t;

/**
 * Defines functionality of a weapon.
 */
class WeaponFunction {
public:

	WeaponFunction(float damage, bool isSecondary = false);

	virtual ~WeaponFunction() {
	}

	virtual void attack(Buttons& buttons, float distance) const;

	virtual Vector getAim(const Vector& target, const Vector& eye) const;

	bool isExplosive() const {
		return explosive;
	}

	void setExplosive(bool explosive) {
		this->explosive = explosive;
	}

	bool isFullAuto() const {
		return fullAuto;
	}

	void setFullAuto(bool fullAuto) {
		this->fullAuto = fullAuto;
	}

	bool isMelee() const {
		return melee;
	}

	void setMelee(bool melee) {
		this->melee = melee;
	}

	void setClipId(int id) {
		clipId = id;
	}

	int getAmmo(edict_t* player) const;

	float getDamageRating(edict_t* player, float dist) const {
		return dist < 0.0f || (isInRange(dist) && (clipId <= -1 || getAmmo(player) > 0.0f)) ? damageRating: 0.0f;
	}

	void setDamageRating(float damageRating) {
		this->damageRating = damageRating;
	}

	float* getRange() {
		return range;
	}

	/**
	 * Assumes that any range below 0 is to be ignored and assumed to be in range.
	 * @return True if a target is within range.
	 */
	bool isInRange(float dist) const {
		return dist <= 0.0f || (range[0] < dist && range[1] > dist);
	}

	void setClip(int* clip) {
		this->clip = clip;
	}

	int getClip() const {
		return clip == nullptr ? 0 : *clip;
	}

	void setSilent(bool silent) {
		this->silent = silent;
	}

	bool isSilent() const {
		return silent;
	}

protected:
	int clipId, button, *clip = nullptr;

	/**
	 * Amount of damage from 0 to 1.  Used for weapon selection.
	 * TODO: consider parsing weapon file information.
	 */
	float damageRating, range[2];

	bool explosive, melee, fullAuto, silent = false;

};
