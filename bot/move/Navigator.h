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
	 * Get the current area the entity is in.
	 */
	static CNavArea* getArea(edict_t* ent);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	bool step();

	bool reachedGoal() const;

	void start(CUtlStack<CNavArea*>* path, const Vector& goal, float targetRadius);

	bool buildPath(const Vector& targetLoc, CUtlStack<CNavArea*>& path);

	CNavArea* getLastArea() const {
		return lastArea;
	}

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

	CNavArea* getCurrentArea(const Vector& pos) const;

	/**
	 * Checks to see if a ladder is required for traversing the two areas.
	 * If a ladder is required, moveCtx is updated accordingly.
	 * @return True if a ladder is required.
	 */
	bool findLadder(const CNavArea* from, const CNavArea* to);
};
