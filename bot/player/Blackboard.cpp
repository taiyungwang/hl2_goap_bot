#include "Blackboard.h"

#include <player/Button.h>
#include <player/Bot.h>
#include <move/Navigator.h>
#include <util/BasePlayer.h>
#include <util/EntityUtils.h>

Blackboard::Blackboard(Bot *self, BasePlayer *entInstance) :
		self(self), entInstance(entInstance) {
	reset();
}

void Blackboard::reset() {
	blocker = nullptr;
}

Blackboard::~Blackboard() {
	delete entInstance;
	reset();
}

bool Blackboard::isOnLadder() {
	return entInstance->isOnLadder();
}

