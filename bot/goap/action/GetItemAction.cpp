#include "GetItemAction.h"

#include <util/EntityVar.h>
#include <const.h>

bool GetItemAction::isAvailable(edict_t* ent) const {
	return (availability.get<int>(ent) & EF_NODRAW) == 0;
}

bool GetItemAction::execute() {
	GoToItemAction::execute();
	return true;
}
