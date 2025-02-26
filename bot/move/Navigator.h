#pragma once

#include <utlvector.h>
#include <vector>

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
	using Path = std::vector<std::pair<int, int>>;

	/**
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent, int team);

	static CNavArea* getArea(const Vector& pos, int team);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	/**
	 * Takes a step on the nav mesh path.
	 * @return True if the bot cannot move further along the given path.
	 */
	bool step();

	/**
	 * @return True if the bot has reached the given goal
	 */
	bool reachedGoal() const;

	/**
	 * @param finalGoal Position of the goal
	 * @param targetRadius Radius of the goal
	 * @param sprint Whether the bot should be sprinting while navigating this path.
	 */
	void start(const Vector& finalGoal, float targetRadius, bool sprint);

	Path &getPath() {
		return path;
	}

protected:
	Blackboard& blackboard;

	bool canMoveTo(const Vector& start, Vector to, float targetRadius, bool crouch) const;

	virtual bool checkCanMove();

private:
	Vector finalGoal;

	Path path;

	int lastAreaId = -1, areaTime = 0;

	MoveStateContext* moveCtx;

	float targetRadius = 0.0f;

	bool sprint = false;

	bool canGetNextArea(const Vector& loc);

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

	void lookAtFurthestVisibleArea() const;
};
