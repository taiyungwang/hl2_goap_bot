#pragma once

class Blackboard;
class Vector;
struct edict_t;

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target);

class Vision {
public:
	static void updateVisiblity(Blackboard& blackboard);
};
