#pragma once

#include <util/EntityInstance.h>

class DODObjectiveResource: public EntityInstance {
public:
	DODObjectiveResource();

	int* getOwner() {
		return getPtr<int>("m_iOwner");
	}

	int numCtrlPts() {
		return get<int>("m_iNumControlPoints");
	}

	Vector* getCapturePositions() {
		return getPtr<Vector>("m_vCPPositions");
	}
};
