#pragma once

#include <iserverplugin.h>

class PluginAdaptor;
class KeyValues;
class CCommand;
struct edict_t;

class VSPlugin: public IServerPluginCallbacks {
public:
	VSPlugin();

	virtual ~VSPlugin() {
	}

	// IServerPluginCallbacks methods
	bool Load(CreateInterfaceFn interfaceFactory,
			CreateInterfaceFn gameServerFactory);

	void Unload(void);

	void Pause(void) {
	}

	void UnPause(void) {
	}

	const char *GetPluginDescription(void);

	void LevelInit(char const *pMapName);

	void ServerActivate(edict_t *pEdictList, int edictCount,
			int clientMax) {
	}

	void GameFrame(bool simulating);

	void LevelShutdown(void);

	void ClientActive(edict_t *pEntity) {
	}

	void ClientDisconnect(edict_t *pEntity);

	void ClientPutInServer(edict_t *pEntity, char const *playername);

	void SetCommandClient(int index) {
		m_iClientCommandIndex = index;
	}
	void ClientSettingsChanged(edict_t *pEdict) {
	}
	PLUGIN_RESULT ClientConnect(bool *bAllowConnect, edict_t *pEntity,
			const char *pszName, const char *pszAddress, char *reject,
			int maxrejectlen) {
		// store client info
		return PLUGIN_CONTINUE;
	}

	PLUGIN_RESULT ClientCommand(edict_t *pEntity,
			const CCommand &args);

	PLUGIN_RESULT NetworkIDValidated(const char *pszUserName,
			const char *pszNetworkID) {
		return PLUGIN_CONTINUE;
	}
	void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie,
			edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus,
			const char *pCvarName, const char *pCvarValue) {
	}

	void OnEdictAllocated(edict_t *edict) {
	}

	void OnEdictFreed(const edict_t *edict) {
	}

	int GetCommandIndex() {
		return m_iClientCommandIndex;
	}
private:
	int m_iClientCommandIndex;
	PluginAdaptor* adaptor;
};
