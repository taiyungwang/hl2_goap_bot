#pragma once

#include "FindCoverAction.h"

class FindCoverFromGrenadesAction: public FindCoverAction {
public:
	FindCoverFromGrenadesAction(Blackboard &blackboard);

private:
	void setAvoidAreas() override;

	void getAvoidPosition(Vector &pos, edict_t *avoid) const override;

	bool waitInCover() const override;
};
