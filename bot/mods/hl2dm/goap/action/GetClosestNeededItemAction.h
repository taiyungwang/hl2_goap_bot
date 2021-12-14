#pragma once

#include <goap/action/GoToEntityAction.h>
#include <move/NavMeshPathBuilder.h>
#include <memory>

class Item;
class ItemMap;

class GetClosestNeededItemAction: public GoToEntityAction, private NavMeshPathBuilder {
public:
	static void setItemMap(const ItemMap *m) {
		itemMap = m;
	}

	GetClosestNeededItemAction(Blackboard& blackboard);

	bool onPlanningFinished() {
		return true;
	}

	bool execute() override;

private:
	static const ItemMap *itemMap;

	std::shared_ptr<Item> resource;

	void selectItem() override;

	float getHeuristicCost(CNavArea *area) const override;

	bool foundGoal(CNavArea *area) override;
};
