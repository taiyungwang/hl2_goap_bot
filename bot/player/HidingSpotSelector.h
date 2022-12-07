#pragma once

#include "CommandHandler.h"
#include <igameevents.h>
#include <vector.h>
#include <map>
#include <string>

class Bot;
class CNavArea;

/**
* Chooses a Hiding spot using Thompson sampling.
**/
class HidingSpotSelector: public CommandHandler::Receiver, public IGameEventListener2 {
public:
	HidingSpotSelector(CommandHandler& commandHandler);

	bool receive(edict_t *sender, const CCommand &command) override;

	void FireGameEvent(IGameEvent* event);

	/**
	 * @return -1 if no positions are available.
	 */
	int select(Vector& pos, int team) const;

	void setInUse(int spot, int team, bool inUse);

	void update(int spot, int team, bool success);

	const Vector& getSpotPos(int idx) const {
		return spots.at(static_cast<unsigned int>(idx)).pos;
	}

	const bool isInUse(int idx, int team) const {
		return spots.at(static_cast<unsigned int>(idx)).score[team - 2].inUse;
	}

	void save();

private:
	static std::string getNavFileName();

	static std::string getHidingSpotFileName();

	struct Spot {
		Vector pos;
		struct Team {
			bool inUse = false;
			float success = 0.01f,
				fail = 0.01f;
		} score[2];
	};

	int navFileTimeStamp;

	std::map<unsigned int, Spot> spots;

	void buildFromNavMesh();
};
