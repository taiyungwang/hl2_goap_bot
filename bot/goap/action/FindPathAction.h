#pragma once

#include "Action.h"
#include <utlstack.h>
#include <vector.h>

class Player;
class CNavArea;
struct edict_t;

class FindPathAction: public Action {
public:
	static CNavArea* getArea(edict_t* ent);

	FindPathAction(Blackboard& blackboard) :
			Action(blackboard) {
	}

	virtual float getCost() const {
		return path.Count();
	}

	virtual void init();

protected:

	Vector targetLoc;

	CUtlStack<CNavArea*> path;

	void build();
};
