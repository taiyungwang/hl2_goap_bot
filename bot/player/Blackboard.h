#pragma once

#include <weapon/Armory.h>
#include "Buttons.h"
#include <eiface.h>
#include <iplayerinfo.h>
#include <utlstack.h>

class EntityInstance;
class Player;
class CNavArea;
struct edict_t;

class Blackboard {
public:

	static float clamp180(float angle);

	Blackboard(const CUtlMap<int, Player*>& players, Player* player,
			EntityInstance* entInstance);

	void reset();

	~Blackboard();

	void setTargetLocation(const Vector& loc);

	const Vector& getTargetLocation() const {
		return targetLocation;
	}

	Buttons& getButtons() {
		return buttons;
	}

	void setViewTarget(const Vector& target) {
		this->viewTarget = target;
	}

	const Vector& getViewTarget() const {
		return viewTarget;
	}

	bool isOnLadder();

	float getTargetRadius() const {
		return targetRadius;
	}

	void setTargetRadius(float targetRadius) {
		this->targetRadius = targetRadius;
	}

	float getAimAccuracy(const Vector& pos) const;

	float getTargetEntDistance() const;

	const Player* getSelf() const {
		return self;
	}

	const Player* getTargetedPlayer() const {
		return targetedPlayer;
	}

	void setTargetedPlayer(const Player* targetedPlayer) {
		this->targetedPlayer = targetedPlayer;
	}

	edict_t* getBlocker() const {
		return blocker;
	}

	void setBlocker(edict_t* blocker) {
		this->blocker = blocker;
	}

	const CUtlMap<int, Player*>& getPlayers() const {
		return players;
	}

	CUtlVector<Player*>& getVisibleEnemies() {
		return visibleEnemies;
	}

	CBotCmd& getCmd() {
		return cmd;
	}

	CUtlStack<CNavArea*>* getPath() const {
		return path;
	}

	void setPath(CUtlStack<CNavArea*>* path) {
		this->path = path;
	}

	Armory& getArmory() {
		return armory;
	}

private:

	Armory armory;

	const Player* self, *targetedPlayer;

	CUtlStack<CNavArea*>* path = nullptr;

	CUtlVector<Player*> visibleEnemies;

	const CUtlMap<int, Player*>& players;

	CBotCmd cmd;

	edict_t* blocker;

	float targetRadius;

	Vector targetLocation, viewTarget;

	Buttons buttons;

	EntityInstance* entInstance;
};
