#include "MMSPlugin.h"
#include "bot/PluginAdaptor.h"
#include "player/Bot.h"
#include <shareddefs.h>
#include <usercmd.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <filesystem.h>
#include <ivdebugoverlay.h>
#include <IEngineTrace.h>
#include <igameevents.h>
#include <datacache/imdlcache.h>
#include <vphysics_interface.h>

IBotManager *botmanager = nullptr;
IVDebugOverlay *debugoverlay = nullptr;
IVEngineServer *engine = nullptr;
IServerGameDLL *servergamedll = nullptr;
IEngineTrace *enginetrace = nullptr;
IServerPluginHelpers *helpers = nullptr;
IPlayerInfoManager *playerinfomanager = nullptr;
IFileSystem *filesystem;
IGameEventManager2 *gameeventmanager = nullptr;
IMDLCache *mdlcache = nullptr;
IServerGameClients *gameclients = nullptr;
IPhysicsSurfaceProps *physprops = nullptr;
IVModelInfo *modelinfo = nullptr;
IPhysicsSurfaceProps *physprop = nullptr;
IServerGameEnts *servergameents = nullptr;
ICvar *cVars = nullptr;
CGlobalVars *gpGlobals = nullptr;
MMSPlugin metaModPlugin;

PLUGIN_GLOBALVARS();

/**
 * Something like this is needed to register cvars/CON_COMMANDs.
 */
class BaseAccessor: public IConCommandBaseAccessor {
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase) {
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const*,
		char const*, char const*, char const*, bool, bool);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0,
		edict_t*);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0,
		edict_t*, char const*);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent*,
		bool);
#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t*,
		const CCommand&);
#else
SH_DECL_HOOK1_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *);
#endif
SH_DECL_MANUALHOOK2_void(MHook_PlayerRunCmd, 0, 0, 0, CUserCmd*, IMoveHelper*);

#define ENGINE_CALL(func) SH_CALL(engine, &IVEngineServer::func)

PLUGIN_EXPOSE(MMSPlugin, metaModPlugin);

bool MMSPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen,
		bool late) {
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, enginetrace, IEngineTrace,
			INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer,
			INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameeventmanager, IGameEventManager2,
			INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers,
			INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, cVars, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, physprops, IPhysicsSurfaceProps,
			VPHYSICS_SURFACEPROPS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, modelinfo, IVModelInfo,
			VMODELINFO_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, mdlcache, IMDLCache,
			MDLCACHE_INTERFACE_VERSION);
	if (!engine->IsDedicatedServer()) {
		GET_V_IFACE_CURRENT(GetEngineFactory, debugoverlay, IVDebugOverlay,
				VDEBUG_OVERLAY_INTERFACE_VERSION);
	}
	GET_V_IFACE_ANY(GetEngineFactory, filesystem, IFileSystem,
			FILESYSTEM_INTERFACE_VERSION)
	GET_V_IFACE_ANY(GetServerFactory, servergameents, IServerGameEnts,
			INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, servergamedll, IServerGameDLL,
			INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients,
			INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager,
			INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_ANY(GetServerFactory, botmanager, IBotManager,
			INTERFACEVERSION_PLAYERBOTMANAGER);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients,
			INTERFACEVERSION_SERVERGAMECLIENTS);
	gpGlobals = ismm->GetCGlobals();
	META_LOG(g_PLAPI, "Starting plugin.");
	/* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
	if (ismm->GetVSPInfo(NULL) == NULL) {
		ismm->AddListener(this, this);
		ismm->EnableVSPListener();
	}
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, servergamedll, this,
			&MMSPlugin::Hook_LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, servergamedll, this,
			&MMSPlugin::Hook_GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, servergamedll, this,
			&MMSPlugin::Hook_LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this,
			&MMSPlugin::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients,
			this, &MMSPlugin::Hook_ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this,
			&MMSPlugin::Hook_ClientCommand, false);
	adaptor = std::make_shared<PluginAdaptor>();
	if (PluginAdaptor::getPlayerruncommandOffset() > 0) {
		SH_MANUALHOOK_RECONFIGURE(MHook_PlayerRunCmd,
				PluginAdaptor::getPlayerruncommandOffset(), 0, 0);
	}
#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = cVars;
	ConVar_Register(0, &s_BaseAccessor);
#else
	ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif
	ENGINE_CALL(LogPrint)("All hooks started!\n");
	MathLib_Init();
	return true;
}

bool MMSPlugin::Unload(char *error, size_t maxlen) {
	adaptor.reset();
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, servergamedll, this, &MMSPlugin::Hook_LevelInit, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, servergamedll, this, &MMSPlugin::Hook_GameFrame, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, servergamedll, this, &MMSPlugin::Hook_LevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &MMSPlugin::Hook_ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &MMSPlugin::Hook_ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &MMSPlugin::Hook_ClientCommand, false);
	return true;
}

bool MMSPlugin::Hook_LevelInit(const char *pMapName,
		char const *pMapEntities, char const *pOldLevel,
		char const *pLandmarkName, bool loadGame, bool background) {
	adaptor->levelInit(pMapName);
	return true;
}

void MMSPlugin::Hook_GameFrame(bool simulating) {
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */
	adaptor->getNewPlayers().remove_if(
			[this](edict_t *pEntity) {
				if (dynamic_cast<Bot*>(Player::getPlayer(pEntity)) != nullptr) {
					if (PluginAdaptor::getPlayerruncommandOffset()) {
						SH_ADD_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd,
								pEntity->GetUnknown()->GetBaseEntity(), this,
								&MMSPlugin::Hook_PlayerRunCmd, false);
					}
					return true;
				}
				return false;
			});
	adaptor->gameFrame(simulating);
}

void MMSPlugin::Hook_LevelShutdown() {
	adaptor->levelShutdown();
}

void MMSPlugin::Hook_ClientDisconnect(edict_t *pEntity) {
	if (PluginAdaptor::getPlayerruncommandOffset() > 0
			&& dynamic_cast<Bot*>(Player::getPlayer(pEntity)) != nullptr) {
		SH_REMOVE_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd, pEntity, this,
			&MMSPlugin::Hook_PlayerRunCmd, false);
	}
	adaptor->clientDisconnect(pEntity);
}

void MMSPlugin::Hook_ClientPutInServer(edict_t *pEntity,
		char const *playername) {
	adaptor->clientPutInServer(pEntity);
}

void MMSPlugin::Hook_ClientCommand(edict_t *pEntity, const CCommand &args) {
	adaptor->clientCommand(pEntity, args);
}

void MMSPlugin::Hook_PlayerRunCmd(CUserCmd *pCmd, IMoveHelper *moveHelper) {
	Bot *bot = dynamic_cast<Bot*>(Player::getPlayer(
			servergameents->BaseEntityToEdict(META_IFACEPTR(CBaseEntity))));
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
	RETURN_META(MRES_IGNORED);
}
