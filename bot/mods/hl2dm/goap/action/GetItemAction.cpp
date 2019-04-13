#include "GetItemAction.h"

#include <util/BaseEntity.h>

bool GetItemAction::isAvailable(edict_t* ent) {
	return !BaseEntity(ent).isDestroyedOrUsed();
}
