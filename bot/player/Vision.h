#pragma once

class Blackboard;
class Vector;
struct edict_t;

class CGameTrace;

typedef CGameTrace trace_t;

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target);

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target, trace_t& result);

class Vision {
public:
	static void updateVisiblity(Blackboard& blackboard);
};
