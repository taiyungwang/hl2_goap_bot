#include "DODObjectiveResource.h"

#include <util/EntityUtils.h>
const char* name = "CDODObjectiveResource";

DODObjectiveResource::DODObjectiveResource() :
		EntityInstance(findEntWithSubStrInNetClassName(name), name) {
}
