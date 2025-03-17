#pragma once

#include <bot/goap/action/GoToEntityAction.h>
#include <bot/player/CommandHandler.h>

class GiveAmmoAction: public GoToEntityAction, public CommandHandler::Receiver {
public:
	GiveAmmoAction(Bot *self, CommandHandler &commandHandler);

	bool init();

	bool goalComplete() override;

	bool receive(edict_t *sender, const CCommand&) override;

private:
	void selectItem() {
	}
};
