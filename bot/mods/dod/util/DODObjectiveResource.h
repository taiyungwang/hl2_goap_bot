#pragma once

#include <util/BaseEntity.h>

class DODObjectiveResource: public BaseEntity {
public:
	DODObjectiveResource(edict_t* ent) : BaseEntity(ent) {
	}

	int* getOwner() {
		return getPtr<int>("m_iOwner");
	}

	int numCtrlPts() {
		return get<int>("m_iNumControlPoints");
	}

	Vector* getCapturePositions() {
		return getPtr<Vector>("m_vCPPositions");
	}

	int* getNumBombsRequired() {
		return getPtr<int>("m_iBombsRequired");
	}

	bool* getBombIsPlanted() {
		return getPtr<bool>("m_bBombPlanted");
	}
};
