#pragma once

class Blackboard;

class Vision {
public:
	void updateVisiblity(Blackboard& blackboard);

private:
	unsigned int memoryDur = 0;
};
