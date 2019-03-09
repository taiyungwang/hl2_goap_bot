#pragma once

#include <utlvector.h>
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
	int select(Vector& pos, int team);

	void resetInUse(int spot, int team) {
		spots[spot].score[team > 1 ? team - 2 : 0].inUse = false;
	}

	void update(int spot, int team, bool success);

private:
	struct Spot {
		Vector pos;
		struct Team {
			bool inUse = false;
			float success = 0.01f,
				fail = 0.01f;
		} score[2];
	};

	CUtlVector<Spot> spots;
};
