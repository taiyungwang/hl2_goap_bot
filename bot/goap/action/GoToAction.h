#pragma once

#include "Action.h"
#include <navmesh/nav_ladder.h>
#include <utlstack.h>

class MoveStateContext;
class CNavArea;

/**
 * Defines the action for going to a location on the map.
 */
class GoToAction: public Action {
public:

	/**
	 * Gets the current area the agent is in.
	 */
	static CNavArea* getCurrentArea(const Vector& pos);

	GoToAction(Blackboard& blackboard);

	~GoToAction();

	bool execute();

	void init();

	bool postCondCheck();

	bool isInterruptable() const {
		return true;
	}

protected:
	virtual bool canMove();

private:
	static bool isConnectionOnFloor(const CNavArea* from, const CNavArea* to);

	Vector currentGoal;

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

	/**
	 * Sets the next goal point.
	 */
	void setDestination(const Vector& dest);
};
