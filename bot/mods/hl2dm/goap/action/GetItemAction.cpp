#include "GetItemAction.h"

#include <util/BaseEntity.h>

bool GetItemAction::isAvailable(edict_t* ent) const {
	return !BaseEntity(ent).isDestroyedOrUsed();
}
