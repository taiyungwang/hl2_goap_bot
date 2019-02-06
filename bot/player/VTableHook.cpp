#include "VTableHook.h"

#include "Bot.h"
#include "PlayerManager.h"
#include <util/SimpleException.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <shareddefs.h>
#include <usercmd.h>
#include <utlstring.h>
#ifndef _WIN32
#include <sys/mman.h>
#else 
#include <Windows.h>
#include <winnt.h>
#endif

DWORD* player_vtable = nullptr;

DWORD VirtualTableHook(DWORD* pdwNewInterface, int vtable, DWORD newInterface) {
	DWORD dwStor = pdwNewInterface[vtable], dwStorVal =
			reinterpret_cast<DWORD>(&pdwNewInterface[vtable]);
#ifdef _WIN32
	DWORD dwOld;
	VirtualProtect( &pdwNewInterface[vtable], 4, PAGE_EXECUTE_READWRITE, &dwOld );
#else
	DWORD alignOffset = dwStorVal % sysconf(_SC_PAGE_SIZE);
	// need page aligned address
	char *addr = reinterpret_cast<char *>(dwStorVal - alignOffset);
	int len = sizeof(DWORD) + alignOffset;
	if (mprotect(addr, len, PROT_EXEC | PROT_READ | PROT_WRITE) == -1) {
		throw SimpleException(CUtlString(
				"In VirtualTableHook while calling mprotect for write access: ")
				+ strerror(errno));
	}
#endif
	*reinterpret_cast<DWORD*>(dwStorVal) = newInterface;
#ifdef _WIN32
	VirtualProtect(&pdwNewInterface[vtable], 4, dwOld, &dwOld);
#else
	if (mprotect(addr, len, PROT_EXEC | PROT_READ) == -1) {
		throw SimpleException(CUtlString(
				"In VirtualTableHook while calling mprotect to remove write access: ")
				+ strerror(errno));
	}
#endif
	return dwStor;
}

void (CBaseEntity::*pPlayerRunCommand)(CUserCmd*, IMoveHelper*) = nullptr;

static void nPlayerRunCommand(CBaseEntity *_this, CUserCmd* pCmd,
		IMoveHelper* pMoveHelper) {
	extern IServerGameEnts *servergameents;
	edict_t *pEdict = servergameents->BaseEntityToEdict(_this);
	extern PlayerManager *playerManager;
	Bot* bot = dynamic_cast<Bot*>(playerManager->getPlayer(
			servergameents->BaseEntityToEdict(_this)));
	if (bot != nullptr) {
		auto cmd = bot->getCmd();
		if (cmd == nullptr) {
			return;
		}
		// put the bot's commands into this move frame
		pCmd->buttons = cmd->buttons;
		pCmd->forwardmove = cmd->forwardmove;
		pCmd->impulse = cmd->impulse;
		pCmd->sidemove = cmd->sidemove;
		pCmd->upmove = cmd->upmove;
		pCmd->viewangles = cmd->viewangles;
		pCmd->weaponselect = cmd->weaponselect;
		pCmd->weaponsubtype = cmd->weaponsubtype;
		pCmd->tick_count = cmd->tick_count;
		pCmd->command_number = cmd->command_number;
	}
	(_this->*pPlayerRunCommand)(pCmd, pMoveHelper);
}

static ConVar mybot_dod_playerruncommand_offset(
		"mybot_dod_playerruncommand_offset", "418");

int getPlayerRunCommandOffset() {
	int vtable = mybot_dod_playerruncommand_offset.GetInt();
#ifndef _WIN32
	vtable++;
#endif
	return vtable;

}

void hookPlayerRunCommand(edict_t *edict) {
	IServerUnknown* unk = edict->GetUnknown();
	if (unk == nullptr) {
		throw SimpleException(
				"Could not get unknown in HookRunPlayerRunCommand.");
	}
	CBaseEntity *BasePlayer = unk->GetBaseEntity();
	if (BasePlayer
			&& pPlayerRunCommand == nullptr) {
		player_vtable =
				reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(BasePlayer));
		*reinterpret_cast<DWORD*>(&pPlayerRunCommand) = VirtualTableHook(
				player_vtable, getPlayerRunCommandOffset(),
				reinterpret_cast<DWORD>(nPlayerRunCommand));
	}
}

// end hook
void unhookPlayerRunCommand() {
	if (pPlayerRunCommand && player_vtable) {
		VirtualTableHook(player_vtable, getPlayerRunCommandOffset(),
				*reinterpret_cast<DWORD*>(&pPlayerRunCommand));
		//pPlayerRunCommandHookedClass = NULL;
		pPlayerRunCommand = nullptr;
	}
}
