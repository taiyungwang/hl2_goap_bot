#pragma once

#include <igameevents.h>
#include <initializer_list>

class GameEventListener: public IGameEventListener2 {
public:
	~GameEventListener();

protected:
	void listenForGameEvent(std::initializer_list<const char*> events);
};
