#pragma once

#include <set>
#include <string>

class Bot;
class Vector;
struct edict_t;

class Vision {
public:
	Vision();

	void reset();

	void updateVisiblity(Bot *self);

	const std::set<int>& getVisibleEnemies() const {
		return visibleEnemies;
	}

	const std::set<int>& getNearbyTeammates() const {
		return nearByTeammates;
	}

	int getTargetedPlayer() const {
		return targetedPlayer;
	}

	void setMiniMapRange(float miniMapRange) {
		this->miniMapRange = miniMapRange;
	}

	void addClassName(const char* name) {
		classNames.insert(name);
	}

	const std::set<int> &getVisibleEntities() const {
		return visibleEntities;
	}

private:
	unsigned int memoryDur = 0;

	std::set<int> visibleEnemies, nearByTeammates, visibleEntities;

	int targetedPlayer = 0;

	std::set<std::string> classNames;

	float miniMapRange;
};
