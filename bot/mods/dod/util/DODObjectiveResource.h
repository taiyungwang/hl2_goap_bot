#pragma once

#include <util/BaseEntity.h>

class DODObjectiveResource: public BaseEntity {
public:
	DODObjectiveResource(edict_t* ent) : BaseEntity(ent) {
	}

	int* getOwner() {
		return getPtr<int>("m_iOwner");
	}

	int *numCtrlPts() const {
		return getPtr<int>("m_iNumControlPoints");
	}

	Vector* getCapturePositions() const {
		return getPtr<Vector>("m_vCPPositions");
	}

	int* getNumBombsRequired() const {
		return getPtr<int>("m_iBombsRequired");
	}

	bool* getBombIsPlanted() const {
		return getPtr<bool>("m_bBombPlanted");
	}
};
