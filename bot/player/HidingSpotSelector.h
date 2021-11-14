#pragma once

#include "CommandHandler.h"
#include <vector.h>
#include <map>
#include <string>

class CNavArea;

/**
* Chooses a Hiding spot using Thompson sampling.
**/
class HidingSpotSelector: public CommandHandler::Receiver {
public:
	HidingSpotSelector(CommandHandler& commandHandler, const std::string& modName);

	bool receive(edict_t *sender, const CCommand &command) override;

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

	void save(const std::string& modName);

private:
	static std::string getNavFileName();

	static std::string getHidingSpotFileName(const std::string& modName);

	struct Spot {
		Vector pos;
		struct Team {
			bool inUse = false;
			float success = 0.01f,
				fail = 0.01f;
		} score[2];
	};

	std::map<unsigned int, Spot> spots;

	void buildFromNavMesh();
};
