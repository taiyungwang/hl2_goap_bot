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
	static CNavArea* getArea(edict_t* ent, int team);

	Navigator(Blackboard& blackboard);

	virtual ~Navigator();

	/**
	 * Takes a step on the nav mesh path.
	 * @return True if the bot has reached its goal.
	 */
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
	bool getPortalToNextArea(Vector& portal) const;

	Vector finalGoal;

	CUtlStack<CNavArea*>* path = nullptr;

	CNavArea *buildPathStartArea = nullptr, *lastArea = nullptr;

	MoveStateContext* moveCtx;

	float targetRadius = 25.0f;

	bool touchedAreaCenter = false;

	bool getNextArea(const Vector& loc, const CNavArea* area);

	bool canMoveTo(Vector to) const;

	CNavArea* getCurrentArea(const Vector& pos) const;

	/**
	 * Finds the start for a ladder to the next area.  Assumes that getPortalToNextArea()
	 * was called to see if the two areas does not require traversing a ladder.
	 */
	void setLadderStart();
};
