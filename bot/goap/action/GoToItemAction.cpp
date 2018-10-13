#include "GoToItemAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/EntityUtils.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>
#include <edict.h>
#include <in_buttons.h>

extern EntityClassManager *classManager;

GoToItemAction::GoToItemAction(const char* name, const char* className,
		const char* classVar, Blackboard& blackboard) :
		availability(classManager->getClass(className)->getEntityVar(classVar)),
		GoToEntityAction(blackboard, name) {
}
