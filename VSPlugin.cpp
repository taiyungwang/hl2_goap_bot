/*
 *  Created on: Dec 12, 2016
 */

#include "VSPlugin.h"

#include "bot/PluginAdaptor.h"
#include "player/Bot.h"
#include <IEngineTrace.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <filesystem.h>
#include <tier1.h>
#include <iplayerinfo.h>
#include <icvar.h>
#include <vphysics_interface.h>
#include <datacache/imdlcache.h>
#include <usercmd.h>
#include <string>
#ifndef _WIN32
#include <sys/mman.h>
#else
#include <Windows.h>
#include <winnt.h>
#endif

IBotManager *botmanager = nullptr;
IVDebugOverlay *debugoverlay = nullptr;
IVEngineServer* engine = nullptr;
IServerGameDLL *servergamedll = nullptr;
IEngineTrace *enginetrace = nullptr;
IServerPluginHelpers *helpers = nullptr;
IPlayerInfoManager *playerinfomanager = nullptr;
IFileSystem *filesystem;
IGameEventManager2 *gameeventmanager = nullptr;
IGameEventManager *gameeventmanager1 = nullptr;
IMDLCache *mdlcache = nullptr;
IServerGameClients* gameclients = nullptr;
IPhysicsSurfaceProps *physprops = nullptr;
IVModelInfo *modelinfo = nullptr;
IPhysicsSurfaceProps *physprop = nullptr;
IServerGameEnts *servergameents = nullptr;
ICvar* cVars = nullptr;
CGlobalVars *gpGlobals = nullptr;

//
// The plugin is a static singleton that is exported as an interface
//
VSPlugin plugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VSPlugin, IServerPluginCallbacks,
		INTERFACEVERSION_ISERVERPLUGINCALLBACKS, plugin);

template<typename type>
bool loadUndefined(type*& var, CreateInterfaceFn gameServerFactory,
		const char* vername, unsigned int maxver, unsigned int minver) {
	char str[256];
	for (unsigned int ver = maxver; ver >= minver && var == nullptr; ver--) {
		memset(str, 0, 256);
		sprintf(str, "%s%03d", vername, ver);
		var = reinterpret_cast<type*>(gameServerFactory(str, nullptr));
		if (var != nullptr) {
			Msg("Found interface %s\n", str);
			return true;
		}
	}
	Warning("Cannot open interface %s (Max ver: %d) (Min ver: %d) \n", vername,
			maxver, minver);
	return false;
}

template<typename T>
bool load(T*& ptr, CreateInterfaceFn interfaceFactory, const char *ver) {
	ptr = reinterpret_cast<T*>(interfaceFactory(ver, nullptr));
	if (ptr == nullptr) {
		Warning("Unable to load %s.\n", ver);
		return false;
	}
	return true;
}

VSPlugin::VSPlugin() {
	adaptor = nullptr;
	m_iClientCommandIndex = 0;
}

bool VSPlugin::Load(CreateInterfaceFn interfaceFactory,
		CreateInterfaceFn gameServerFactory) {
	ConnectTier1Libraries(&interfaceFactory, 1);
	if (!load(mdlcache, interfaceFactory, MDLCACHE_INTERFACE_VERSION)
			|| !load(physprops, interfaceFactory, VPHYSICS_SURFACEPROPS_INTERFACE_VERSION)
			|| !load(filesystem, interfaceFactory, FILESYSTEM_INTERFACE_VERSION)
			|| !load(gameeventmanager1, interfaceFactory, INTERFACEVERSION_GAMEEVENTSMANAGER)
			|| !load(gameeventmanager, interfaceFactory, INTERFACEVERSION_GAMEEVENTSMANAGER2)
			|| !load(engine, interfaceFactory, INTERFACEVERSION_VENGINESERVER)
			|| !load(helpers, interfaceFactory, INTERFACEVERSION_ISERVERPLUGINHELPERS)
			|| !load(enginetrace, interfaceFactory, INTERFACEVERSION_ENGINETRACE_SERVER)
			|| !load(modelinfo, interfaceFactory, VMODELINFO_SERVER_INTERFACE_VERSION)
			|| (!engine->IsDedicatedServer() && !load(debugoverlay, interfaceFactory,
					VDEBUG_OVERLAY_INTERFACE_VERSION))
			|| !load(cVars, interfaceFactory, CVAR_INTERFACE_VERSION)
			|| !loadUndefined(playerinfomanager, gameServerFactory, "PlayerInfoManager", 3, 2)
			|| !loadUndefined(gameclients, gameServerFactory, "ServerGameClients", 5, 4)
			|| !loadUndefined(servergamedll, gameServerFactory, "ServerGameDLL", 12, 10)
			|| !loadUndefined(botmanager, gameServerFactory, "BotManager", 3, 1)
			|| !loadUndefined(servergameents, gameServerFactory,
					"ServerGameEnts", 10, 1)) {
		return false;
	}
	gpGlobals = playerinfomanager->GetGlobalVars();
	adaptor = new PluginAdaptor();
	ConVar_Register(0);
	MathLib_Init();
	return true;
}

const char *VSPlugin::GetPluginDescription(void) {
	return adaptor->getPluginDescription();
}

void VSPlugin::LevelInit(char const *pMapName) {
	gameeventmanager->AddListener(this, "MyBot", true);
	gameeventmanager1->AddListener(this, true);
	adaptor->levelInit(pMapName);
}

DWORD VirtualTableHook(DWORD* pdwNewInterface, int vtable, DWORD newInterface) {
	DWORD dwStor = pdwNewInterface[vtable], dwStorVal =
			reinterpret_cast<DWORD>(&pdwNewInterface[vtable]);
#ifdef _WIN32
	DWORD dwOld;
	char buf[256];
	if (!VirtualProtect(&pdwNewInterface[vtable], 4, PAGE_EXECUTE_READWRITE, &dwOld)) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(buf[0])), NULL);
		throw SimpleException(CUtlString(
				"In VirtualTableHook while calling VirtualProtect for write access: ")
				+ buf);
	}
#else
	DWORD alignOffset = dwStorVal % sysconf(_SC_PAGE_SIZE);
	// need page aligned address
	char *addr = reinterpret_cast<char *>(dwStorVal - alignOffset);
	int len = sizeof(DWORD) + alignOffset;
	if (mprotect(addr, len, PROT_EXEC | PROT_READ | PROT_WRITE) == -1) {
		Error(std::string("In VirtualTableHook while calling mprotect for write access: %s").c_str(),
				strerror(errno));
	}
#endif
	*reinterpret_cast<DWORD*>(dwStorVal) = newInterface;
#ifdef _WIN32
	if (!VirtualProtect(&pdwNewInterface[vtable], 4, dwOld, &dwOld)) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(buf[0])), NULL);
		throw SimpleException(CUtlString(
			"In VirtualTableHook while calling VirtualProtect to remove write access: ")
			+ buf);
	}
#else
	if (mprotect(addr, len, PROT_EXEC | PROT_READ) == -1) {
		Error(std::string("In VirtualTableHook while calling mprotect to remove write access: %s").c_str(),
				strerror(errno));
	}
#endif
	return dwStor;
}

void (CBaseEntity::*pPlayerRunCommand)(CUserCmd*, IMoveHelper*) = nullptr;

#ifndef _WIN32
void nPlayerRunCommand(CBaseEntity *_this, CUserCmd* pCmd,
		IMoveHelper* pMoveHelper)
#else
void __fastcall nPlayerRunCommand(CBaseEntity *_this, void*, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
#endif
{
	extern IServerGameEnts *servergameents;
	Bot* bot = dynamic_cast<Bot*>(Player::getPlayer(servergameents->BaseEntityToEdict(_this)));
	if (bot != nullptr) {
		auto cmd = bot->getCmd();
		if (cmd != nullptr) {
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
	}
	(_this->*pPlayerRunCommand)(pCmd, pMoveHelper);
}

void hookPlayerRunCommand(edict_t *edict, int offset) {
	IServerUnknown* unk = edict->GetUnknown();
	if (unk == nullptr) {
		Error("Could not get unknown in HookRunPlayerRunCommand.");
	}
	CBaseEntity *BasePlayer = unk->GetBaseEntity();
	if (BasePlayer
			&& pPlayerRunCommand == nullptr) {
		*reinterpret_cast<DWORD*>(&pPlayerRunCommand) = VirtualTableHook(
				reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(BasePlayer)), offset,
				reinterpret_cast<DWORD>(nPlayerRunCommand));
	}
}

void VSPlugin::GameFrame(bool simulating) {
	const auto& players = Player::getPlayers();
	adaptor->getNewPlayers().remove_if([this, players](edict_t *pEntity) {
		if (players.find(engine->IndexOfEdict(pEntity)) != players.end()) {
			if (adaptor->getHookOffset() > 0) {
				hookPlayerRunCommand(pEntity, adaptor->getHookOffset());
			}
			return true;
		}
		return false;
	});
	adaptor->gameFrame(simulating);
}

void VSPlugin::ClientPutInServer(edict_t *pEntity, char const *playername) {
	adaptor->clientPutInServer(pEntity);
}

void VSPlugin::ClientDisconnect(edict_t *pEntity) {
	adaptor->clientDisconnect(pEntity);
}

void VSPlugin::LevelShutdown(void) {
	adaptor->levelShutdown();
	gameeventmanager->RemoveListener(this);
	gameeventmanager1->RemoveListener(this);
}

void VSPlugin::Unload(void) {
	ConVar_Unregister();
	delete adaptor;
	adaptor = nullptr;
	DisconnectTier1Libraries();
}

PLUGIN_RESULT VSPlugin::ClientCommand(edict_t *pEntity,
		const CCommand &args) {
	adaptor->clientCommand(pEntity, args);
	return PLUGIN_CONTINUE;
}

void VSPlugin::FireGameEvent(IGameEvent *event) {
	adaptor->handEvent(event);
}

// IGameEventListener Interface
void VSPlugin::FireGameEvent(KeyValues *event) {
	adaptor->handEvent(event);
}
