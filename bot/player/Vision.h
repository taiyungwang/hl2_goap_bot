#pragma once

class Blackboard;
class Vector;
struct edict_t;

class CGameTrace;

typedef CGameTrace trace_t;

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target);

bool UTIL_IsVisible(trace_t& result, const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target);

class Vision {
public:
	void updateVisiblity(Blackboard& blackboard);

private:
	unsigned int memoryDur = 0;
};
