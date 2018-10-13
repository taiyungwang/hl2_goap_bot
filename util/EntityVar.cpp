#include "EntityVar.h"

#include "SimpleException.h"
#include <utlstring.h>
#include <eiface.h>
#include <edict.h>

CBaseEntity* EntityVar::getBaseEntity(edict_t* ent) const {
	if (ent == nullptr) {
		return nullptr;
	}
	IServerUnknown* unk = ent->GetUnknown();
	if (unk == nullptr) {
		return nullptr;
	}
	return unk->GetBaseEntity();
}

edict_t* EntityVar::getEntity(edict_t* ent) const {
	CBaseHandle* out = getVarPtr<CBaseHandle>(getBaseEntity(ent));
	extern IVEngineServer* engine;
	return out == nullptr ?
			nullptr : engine->PEntityOfEntIndex(out->GetEntryIndex());
}

void EntityVar::throwException(edict_t* ent) const {
	throw SimpleException(CUtlString("Unable to retrieve entity value with class name, ")
			+ ent->GetClassName() + ".\n");
}
