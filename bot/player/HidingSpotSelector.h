#pragma once

#include <strtools.h>
#include <utlhashtable.h>
#include <vector.h>

class CNavArea;

/**
* Chooses a Hiding spot using Thompson sampling.
**/
class HidingSpotSelector {
public:

	HidingSpotSelector();

	/**
	 * @return -1 if no positions are available.
	 */
	int select(Vector& pos, int team) const;

	void setInUse(int spot, int team, bool inUse);

	void update(int spot, int team, bool success);

	const Vector& getSpotPos(int idx) const {
		return spots.GetPtr(idx)->pos;
	}

	const bool isInUse(int idx, int team) const {
		return spots.GetPtr(idx)->score[team - 2].inUse;
	}

private:
	struct Spot {
		Vector pos;
		struct Team {
			bool inUse = false;
			float success = 0.01f,
				fail = 0.01f;
		} score[2];
	};

	CUtlHashtable<unsigned int, Spot> spots;
};
