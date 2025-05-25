#include "GetClosestNeededItemAction.h"
#include "ItemMap.h"
#include "Item.h"
#include <player/Bot.h>
#include <player/Buttons.h>
#include <move/Navigator.h>
#include <nav_mesh/nav_area.h>
#include <util/BaseEntity.h>
#include <util/UtilTrace.h>

const ItemMap *GetClosestNeededItemAction::itemMap = nullptr;

GetClosestNeededItemAction::GetClosestNeededItemAction(Bot *self) :
		GoToEntityAction(self),
		NavMeshPathBuilder(self->getTeam()) {
	effects = { WorldProp::NEED_ITEM, false };
}

bool GetClosestNeededItemAction::init() {
	auto collideable = resource->getEnt()->GetCollideable();
		targetRadius = (collideable->OBBMaxs().AsVector2D()
				.DistTo(collideable->OBBMins().AsVector2D())) * 0.5f;
	animationCycle = *BaseEntity(item).getPtr<float>("m_flCycle");
	animationCycleUnchangedFrames = 0;
	return GoToEntityAction::init();
}

bool GetClosestNeededItemAction::execute() {
	if (!GoToAction::execute()) {
		return false;
	}
	if (animationCycleUnchangedFrames > 10 || !GoToAction::goalComplete() || !resource->isCharger()
			|| (self->getHealth() > 99 && self->getArmor() > 199)) {
		return true;
	}
	float newCycle = *BaseEntity(item).getPtr<float>("m_flCycle");
	bool active = newCycle != animationCycle;
	useItem(active);
	if (!active) {
		animationCycleUnchangedFrames++;
	}
	animationCycle = newCycle;
	return false;
}

bool GetClosestNeededItemAction::goalComplete() {
	return GoToAction::goalComplete() || (!resource->isAvailable()
			&& self->getCurrentPosition().DistTo(targetLoc) < targetRadius + HalfHumanWidth);
}

void GetClosestNeededItemAction::selectItem() {
	Path path;
	build(path, self->getArea());
	self->getNavigator()->getPath().swap(path);
}

float GetClosestNeededItemAction::getHeuristicCost(CNavArea *area) const  {
	return area->GetCenter().DistTo(self->getCurrentPosition());
}

bool GetClosestNeededItemAction::foundGoal(CNavArea *area) {
	item = nullptr;
	resource = itemMap->getClosestNeededItem(area, *self);
	if (!resource) {
		return false;
	}
	item = resource->getEnt();
	extern ConVar mybot_debug;
	if (mybot_debug.GetBool()) {
		self->consoleMsg(std::string("Getting item: ") + item->GetClassName());
	}
	return true;
}
