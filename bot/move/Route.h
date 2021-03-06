#pragma once

#include <utlstack.h>
#include <vector.h>

class CNavArea;
class MoveStateContext;
struct edict_t;

class Route {
public:
	/**
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent);

	/**
	 * Gets the encompassing or closest area given the position.
	 */
	static CNavArea* getCurrentArea(const Vector& pos, int team = -2);

	Route(MoveStateContext &moveCtx, const Vector &goal, float goalRadius) :
			moveCtx(moveCtx), goal(goal), goalRadius(goalRadius) {
	}

	/**
	 * Builds the path for the given goal.
	 * @return True if path built successfully.
	 */
	bool build();

	/**
	 * @return True if stuck or reached goal.
	 */
	bool step(const Vector &pos, bool isOnLadder);

	bool reachedGoal() const;

private:
	/**
	 * Gets the portal of the to area if it is connected without using a ladder.
	 * @Return True if it is connected without using a ladder.
	 */
	static bool getPortal(Vector& portal, const CNavArea* from, const CNavArea* to);

	CUtlStack<CNavArea*> path;
	CNavArea *lastArea = nullptr;
	Vector goal;
	float goalRadius = 25.0f;
	MoveStateContext &moveCtx;

	/**
	 * Gets the next target area.
	 */
	void getNextArea(const Vector& loc);

	bool canMoveTo(const Vector& loc, Vector to) const;

	/**
	 * Checks to see if a ladder is required for traversing the two areas.
	 * If a ladder is required, moveCtx is updated accordingly.
	 * @return True if a ladder is required.
	 */
	bool findLadder(const CNavArea* from, const CNavArea* to);
};

