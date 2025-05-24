#include "GameEventListener.h"

extern IGameEventManager2 *gameeventmanager;

GameEventListener::~GameEventListener() {
	gameeventmanager->RemoveListener(this);
}

void GameEventListener::listenForGameEvent(
		const std::initializer_list<const char*> events) {
	for (const auto event : events) {
		gameeventmanager->AddListener(this, event, true);
	}
}
