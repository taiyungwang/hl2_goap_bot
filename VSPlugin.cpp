/*
 *  Created on: Dec 12, 2016
 */

#include "VSPlugin.h"

#include <bot/PluginAdaptor.h>
#include <IEngineTrace.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <filesystem.h>
#include <tier1.h>
#include <iplayerinfo.h>
#include <icvar.h>
#include <vphysics_interface.h>
#include <datacache/imdlcache.h>

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

void VSPlugin::GameFrame(bool simulating) {
	adaptor->gameFrame(simulating);
}

void VSPlugin::ClientActive(edict_t *pEntity) {
	adaptor->clientActive(pEntity);
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

