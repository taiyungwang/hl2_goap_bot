#pragma once

#include <list>

class Blackboard;
class Player;

class Vision {
public:
	Vision();

	void updateVisiblity(Blackboard& blackboard);

	const std::list<int>& getVisibleEnemies() const {
		return visibleEnemies;
	}

	const std::list<int>& getNearbyTeammates() const {
		return nearByTeammates;
	}

	int getTargetedPlayer() const {
		return targetedPlayer;
	}

	void setMiniMapRange(float miniMapRange) {
		this->miniMapRange = miniMapRange;
	}

private:
	unsigned int memoryDur = 0;

	std::list<int> visibleEnemies, nearByTeammates;

	int targetedPlayer = 0;

	float miniMapRange;
};
