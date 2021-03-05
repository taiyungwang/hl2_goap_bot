#pragma once

#include <utlvector.h>
#include <shareddefs.h>
#include <utlstack.h>

class Blackboard;
class MoveStateContext;
class CNavArea;
struct edict_t;

/**
 * Handles navigation based on the navmesh.
 */
class Navigator {
public:

	/**
	 * Gets the encompassing or closest area given the position.
	 */
	static CNavArea* getCurrentArea(const Vector& pos, int team = TEAM_ANY);

	/**
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	bool step();

	bool reachedGoal() const;

	void start(CUtlStack<CNavArea*>* path, const Vector& goal, float targetRadius);

	bool buildPath(const Vector& targetLoc, CUtlStack<CNavArea*>& path);

protected:
	Blackboard& blackboard;

	virtual bool checkCanMove();

private:
	/**
	 * Gets the portal of the to area if it is connected without using a ladder.
	 * @Return True if it is connected without using a ladder.
	 */
	static bool getPortal(Vector& portal, const CNavArea* from, const CNavArea* to);

	Vector finalGoal;

	CUtlStack<CNavArea*>* path = nullptr;

	CNavArea *buildPathStartArea = nullptr, *lastArea = nullptr;

	MoveStateContext* moveCtx;

	float targetRadius = 25.0f;

	bool canMoveTo(Vector to) const;
	/**
	 * Gets the next target area.
	 */
	void getNextArea();

	/**
	 * Checks to see if a ladder is required for traversing the two areas.
	 * If a ladder is required, moveCtx is updated accordingly.
	 * @return True if a ladder is required.
	 */
	bool findLadder(const CNavArea* from, const CNavArea* to);
};
