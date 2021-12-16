#include "ItemMap.h"
#include "Item.h"
#include <player/Bot.h>
#include <move/Navigator.h>
#include <nav_mesh/nav_area.h>
#include <util/UtilTrace.h>
#include <eiface.h>

void ItemMap::buildMap() {
	items.clear();
	extern CGlobalVars *gpGlobals;
	extern IVEngineServer* engine;
	for (int i = 1; i < gpGlobals->maxEntities; i ++) {
		edict_t *ent = engine->PEntityOfEntIndex(i);
		if (ent == nullptr || ent->IsFree()) {
			continue;
		}
		auto itr = builders.find(ent->GetClassName());
		if (itr == builders.end()) {
			continue;
		}
		CNavArea *area = Navigator::getArea(UTIL_FindGround(ent->GetCollideable()->GetCollisionOrigin()), 0);
		if (area == nullptr) {
			Warning("Unable to find item, %s, in nav mesh.\n", ent->GetClassName());
			continue;
		}
		if (items.find(area->GetID()) == items.end()) {
			items.emplace(area->GetID(), std::list<std::shared_ptr<Item>>());
		}
		items[area->GetID()].push_back(itr->second->build(ent));
	}
}

std::shared_ptr<Item> ItemMap::getClosestNeededItem(CNavArea *area, const Bot& self) const {
	std::shared_ptr<Item> chosen;
	if (items.find(area->GetID()) == items.end()) {
		// TODO: handle map being edited
		return chosen;
	}
	float closest = INFINITY;
	for (auto item: items.at(area->GetID())) {
		if (!item->isAvailable()) {
			continue;
		}
		edict_t *ent = item->getEnt();
		float dist = self.getCurrentPosition().DistTo(ent->GetCollideable()->GetCollisionOrigin());
		auto builder = builders.at(ent->GetClassName());
		if (dist < closest && builder->need(self)) {
			chosen = item;
		}
	}
	return chosen;
}
