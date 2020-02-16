#include "DODObjectiveResource.h"

#include <util/EntityUtils.h>
const char *name = "CDODObjectiveResource";

DODObjectiveResource::DODObjectiveResource() :
		BaseEntity(name, findEntWithSubStrInNetClassName(name)) {
}
