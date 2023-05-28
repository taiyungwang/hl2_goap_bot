#pragma once

#include <ISmmPlugin.h>
#include <memory>

class CCommand;
class CUserCmd;
class IMoveHelper;
class PluginAdaptor;

class MMSPlugin : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen) {
		return true;
	}
	bool Unpause(char *error, size_t maxlen) {
		return true;
	}
	void AllPluginsLoaded() {
	}
	//IMetamodListener stuff
	void OnVSPListening(IServerPluginCallbacks *iface) {
	}
	//hooks
	void Hook_ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {
	}

	bool Hook_LevelInit(const char *pMapName,
		char const *pMapEntities,
		char const *pOldLevel,
		char const *pLandmarkName,
		bool loadGame,
		bool background);
	void Hook_GameFrame(bool simulating);
	void Hook_LevelShutdown(void);
	void Hook_ClientActive(edict_t *pEntity, bool bLoadGame) {
	}
	void Hook_ClientDisconnect(edict_t *pEntity);
	void Hook_ClientPutInServer(edict_t *pEntity, char const *playername);
	void Hook_SetCommandClient(int index) {
	}
	void Hook_ClientSettingsChanged(edict_t *pEdict) {
	}
	bool Hook_ClientConnect(edict_t *pEntity,
		const char *pszName,
		const char *pszAddress,
		char *reject,
		int maxrejectlen);
#if SOURCE_ENGINE >= SE_ORANGEBOX
	void Hook_ClientCommand(edict_t *pEntity, const CCommand &args);
#else
	void Hook_ClientCommand(edict_t *pEntity);
#endif
public:
	const char *GetAuthor() {
		return "taiyungwang";
	}
	const char *GetName() {
		return "HL2 GOAP Bot";
	}
	const char *GetDescription() {
		return "A bot that supports DODS and HL2DM.";
	}
	const char *GetURL() {
		return "https://github.com/taiyungwang/hl2_goap_bot";
	}
	const char *GetLicense() {
		return "GPL2";
	}

	const char *GetVersion() {
		return "1.3.2";
	}

	const char *GetDate() {
		return __DATE__;
	}

	const char *GetLogTag() {
		return "mybot";
	}

	void Hook_PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper);

private:
	std::shared_ptr<PluginAdaptor> adaptor;
};
