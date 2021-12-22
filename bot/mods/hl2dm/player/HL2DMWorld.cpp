#include "HL2DMWorld.h"

#include <goap/WorldCond.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

void HL2DMWorld::addStates() {
	states[WorldProp::NEED_ITEM] = true;
}
