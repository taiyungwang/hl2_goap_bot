#pragma once

#include <navmesh/nav_ladder.h>
#include <utlstack.h>

class MoveStateContext;
class CNavArea;
class Blackboard;

/**
 * Defines the action for going to a location on the map.
 */
class Navigator {
public:

	static CNavArea* getArea(edict_t* ent);

	/**
	 * Gets the current area the agent is in.
	 */
	static CNavArea* getCurrentArea(const Vector& pos);

	Navigator(Blackboard& blackboard, float targetRadius);

	virtual ~Navigator();

	bool navigate();

	bool buildPath(const Vector& targetLoc);

	bool reachedGoal();

	const CUtlStack<CNavArea*>& getPath() const {
		return path;
	}

private:
	static bool isConnectionOnFloor(const CNavArea* from, const CNavArea* to);

	Vector currentGoal, finalGoal;

	float targetRadius;

	CUtlStack<CNavArea*> path;

	MoveStateContext* moveCtx;

	/**
	 * Gets the next target area.
	 */
	void getNextArea();

	/**
	 * Checks to see if a ladder is required for traversing the two areas.
	 * If a ladder is required, moveCtx is updated accordingly.
	 * @return True if a ladder is required.
	 */
	bool findLadder(const CNavArea* from, const CNavArea* to,
			CNavLadder::LadderDirectionType dir);
};
