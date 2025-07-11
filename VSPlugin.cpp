/*
 *  Created on: Dec 12, 2016
 */

#include "VSPlugin.h"

#include "bot/PluginAdaptor.h"
#include "player/Bot.h"
#include <shareddefs.h>
#include <usercmd.h>
#include <IEngineTrace.h>
#include <ivdebugoverlay.h>
#include <filesystem.h>
#include <tier1.h>
#include <iplayerinfo.h>
#include <icvar.h>
#include <vphysics_interface.h>
#include <datacache/imdlcache.h>
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
IMDLCache *mdlcache = nullptr;
IServerGameClients* gameclients = nullptr;
IPhysicsSurfaceProps *physprops = nullptr;
IVModelInfo *modelinfo = nullptr;
IPhysicsSurfaceProps *physprop = nullptr;
IServerGameEnts *servergameents = nullptr;
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
	if ((ptr = static_cast<T*>(interfaceFactory(ver, nullptr))) == nullptr) {
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
			|| !load(gameeventmanager, interfaceFactory, INTERFACEVERSION_GAMEEVENTSMANAGER2)
			|| !load(engine, interfaceFactory, INTERFACEVERSION_VENGINESERVER)
			|| !load(helpers, interfaceFactory, INTERFACEVERSION_ISERVERPLUGINHELPERS)
			|| !load(enginetrace, interfaceFactory, INTERFACEVERSION_ENGINETRACE_SERVER)
			|| !load(modelinfo, interfaceFactory, VMODELINFO_SERVER_INTERFACE_VERSION)
			|| (!engine->IsDedicatedServer() && !load(debugoverlay, interfaceFactory,
					VDEBUG_OVERLAY_INTERFACE_VERSION))
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

const char *VSPlugin::GetPluginDescription() {
	return PluginAdaptor::getPluginDescription();
}

void VSPlugin::LevelInit(char const *pMapName) {
	adaptor->levelInit(pMapName);
}

uint64_t VirtualTableHook(uint64_t* pdwNewInterface, int vtable, uint64_t newInterface) {
	uint64_t dwStor = pdwNewInterface[vtable],
			dwStorVal = reinterpret_cast<uint64_t>(&pdwNewInterface[vtable]);
#ifdef PLATFORM_WINDOWS_PC
	DWORD dwOld;
	char buf[256];
	if (!VirtualProtect(&pdwNewInterface[vtable], 4, PAGE_EXECUTE_READWRITE, &dwOld)) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(buf[0])), NULL);
		Error("In VirtualTableHook while calling VirtualProtect for write access: %s",
				buf);
	}
#else
	uint64_t alignOffset = dwStorVal % sysconf(_SC_PAGE_SIZE);
	// need page aligned address
	char *addr = reinterpret_cast<char *>(dwStorVal - alignOffset);
	int len = sizeof(uint64_t) + alignOffset;
	if (mprotect(addr, len, PROT_EXEC | PROT_READ | PROT_WRITE) == -1) {
		Error(std::string("In VirtualTableHook while calling mprotect for write access: %s").c_str(),
				strerror(errno));
	}
#endif
	*reinterpret_cast<uint64_t*>(dwStorVal) = newInterface;
#ifdef PLATFORM_WINDOWS_PC
	if (!VirtualProtect(&pdwNewInterface[vtable], 4, dwOld, &dwOld)) {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(buf[0])), NULL);
		Error("In VirtualTableHook while calling VirtualProtect to remove write access: %s",
				buf);
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

void FASTCALL nPlayerRunCommand(CBaseEntity *_this, CUserCmd* pCmd,
		IMoveHelper* pMoveHelper)
{
	extern IServerGameEnts *servergameents;
	Bot* bot = dynamic_cast<Bot*>(Player::getPlayer(servergameents->BaseEntityToEdict(_this)));
	if (bot != nullptr) {
		auto cmd = bot->getCmd();
			// put the bot's commands into this move frame
		pCmd->buttons = cmd.buttons;
		pCmd->forwardmove = cmd.forwardmove;
		pCmd->impulse = cmd.impulse;
		pCmd->sidemove = cmd.sidemove;
		pCmd->upmove = cmd.upmove;
		pCmd->viewangles = cmd.viewangles;
		pCmd->weaponselect = cmd.weaponselect;
		pCmd->weaponsubtype = cmd.weaponsubtype;
		pCmd->tick_count = cmd.tick_count;
		pCmd->command_number = cmd.command_number;
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
		*reinterpret_cast<uint64_t*>(&pPlayerRunCommand) = VirtualTableHook(
				reinterpret_cast<uint64_t*>(*reinterpret_cast<uint64_t*>(BasePlayer)), offset,
				reinterpret_cast<uint64_t>(nPlayerRunCommand));
	}
}

void VSPlugin::GameFrame(bool simulating) {
	adaptor->getNewPlayers().remove_if([](edict_t *pEntity) {
		auto player = Player::getPlayer(pEntity);
		if (dynamic_cast<Bot*>(player) != nullptr) {
			if (PluginAdaptor::getPlayerruncommandOffset() > 0) {
				hookPlayerRunCommand(pEntity, PluginAdaptor::getPlayerruncommandOffset());
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
