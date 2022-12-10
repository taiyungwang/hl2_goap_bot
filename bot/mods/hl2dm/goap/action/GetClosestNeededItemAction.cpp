#include "GetClosestNeededItemAction.h"
#include "ItemMap.h"
#include "Item.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <move/Navigator.h>
#include <nav_mesh/nav_area.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

const ItemMap *GetClosestNeededItemAction::itemMap = nullptr;

GetClosestNeededItemAction::GetClosestNeededItemAction(Blackboard &blackboard) :
		GoToEntityAction(blackboard),
		NavMeshPathBuilder(blackboard.getSelf()->getTeam()) {
	effects = { WorldProp::NEED_ITEM, false };
}

bool GetClosestNeededItemAction::init() {
	if (resource->isCharger()) {
		auto collideable = resource->getEnt()->GetCollideable();
		targetRadius = (collideable->OBBMaxs().AsVector2D()
				.DistTo(collideable->OBBMins().AsVector2D())) * 0.5f;
	} else {
		targetRadius = 0.0f;
	}
	return GoToEntityAction::init();
}

bool GetClosestNeededItemAction::execute() {
	if (!GoToAction::execute()) {
		return false;
	}
	auto self = blackboard.getSelf();
	if (!GoToAction::goalComplete() || !resource->isCharger() || !resource->isAvailable()
			|| (self->getHealth() > 99 && self->getArmor() > 99)) {
		return true;
	}
	useItem();
	return false;
}

void GetClosestNeededItemAction::selectItem() {
	auto self = blackboard.getSelf();
	NavMeshPathBuilder::Path path;
	build(path, self->getArea());
	self->getNavigator()->getPath().swap(path);
}

float GetClosestNeededItemAction::getHeuristicCost(CNavArea *area) const  {
	return area->GetCenter().DistTo(blackboard.getSelf()->getCurrentPosition());
}

bool GetClosestNeededItemAction::foundGoal(CNavArea *area) {
	item = nullptr;
	resource = itemMap->getClosestNeededItem(area, *blackboard.getSelf());
	if (!resource) {
		return false;
	}
	item = resource->getEnt();
	return true;
}
