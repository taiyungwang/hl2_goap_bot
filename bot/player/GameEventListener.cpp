#include "GameEventListener.h"

extern IGameEventManager2 *gameeventmanager;

GameEventListener::~GameEventListener() {
	gameeventmanager->RemoveListener(this);
}

void GameEventListener::listenForGameEvent(
		std::initializer_list<const char*> events) {
	for (auto event : events) {
		gameeventmanager->AddListener(this, event, true);
	}
}
