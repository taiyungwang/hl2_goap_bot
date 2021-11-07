#pragma once

#include <utlvector.h>
#include <shareddefs.h>
#include <stack>

class Blackboard;
class MoveStateContext;
class CNavArea;
class Player;
struct edict_t;

/**
 * Handles navigation based on the navmesh.
 */
class Navigator {
public:
	static CNavArea* getArea(const Player* player);

	/**
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent, int team);

	static CNavArea* getArea(const Vector& pos, int team);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	/**
	 * Takes a step on the nav mesh path.
	 * @return True if the bot has reached its goal.
	 */
	bool step();

	bool reachedGoal() const;

	void start(const Vector& goal, float targetRadius);

	CNavArea* getLastArea() const {
		return lastArea;
	}

protected:
	Blackboard& blackboard;

	virtual bool checkCanMove();

private:
	Vector finalGoal;

	std::stack<CNavArea*> path;

	CNavArea *lastArea = nullptr;

	MoveStateContext* moveCtx;

	float targetRadius = 0.0f;

	int areaTime = 0, dirToTop = 4;

	void setDirToTop();

	bool buildPath();

	bool canGetNextArea(const Vector& loc) const;

	void setGoalForNextArea(const Vector& loc);

	/**
	 * Gets the portal of the to area if it is connected without using a ladder.
	 * @Return True if it is connected without using a ladder.
	 */
	bool getPortalToTopArea(Vector& portal) const;

	bool canMoveTo(Vector to, bool crouch) const;

	/**
	 * Finds the start for a ladder to the next area.  Assumes that getPortalToNextArea()
	 * was called to see if the two areas does not require traversing a ladder.
	 */
	bool setLadderStart();
};
