#pragma once

#include <navmesh/nav_ladder.h>
#include <utlstack.h>

class Blackboard;
class MoveStateContext;
class CNavArea;

/**
 * Handles navigation based on the navmesh.
 */
class Navigator {
public:

	/**
	 * Gets the current area the given position is in.
	 */
	static CNavArea* getCurrentArea(const Vector& pos);

	/**
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	bool step();

	bool reachedGoal() const;

	void start(CUtlStack<CNavArea*>* path, const Vector& goal, 	float targetRadius);

	bool buildPath(const Vector& targetLoc, CUtlStack<CNavArea*>& path);

protected:
	Blackboard& blackboard;

	virtual bool checkCanMove();

private:
	static bool isConnectionOnFloor(const CNavArea* from, const CNavArea* to);

	Vector currentGoal, finalGoal;

	CUtlStack<CNavArea*>* path = nullptr;

	MoveStateContext* moveCtx;

	float targetRadius = 25.0f;

	// Used to track start areas for chained GoToActions.
	CNavArea* startArea = nullptr;

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
