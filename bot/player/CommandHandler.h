#pragma once

#include <list>

class CCommand;
struct edict_t;

#include "ObjectManager.h"

/**
 * Manages the Plugin's Client Commands.
 */
class CommandHandler: public ObjectManager {
public:
	class Receiver: public Object {
	public:
		Receiver(CommandHandler &handler) :
				Object(handler) {
		}

		virtual bool receive(edict_t *sender, const CCommand &command) = 0;
	};

	void handle(edict_t *player, const CCommand &command) {
		for (auto receiver : objects) {
			if (dynamic_cast<Receiver*>(receiver)->receive(player, command)) {
				break;
			}
		}
	}
};
