#pragma once

#include <ISmmPlugin.h>
#include <memory>

class CCommand;
class CUserCmd;
class CBasePlayer;
class IMoveHelper;
class PluginAdaptor;

class MMSPlugin : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) override;
	bool Unload(char *error, size_t maxlen) override;
	bool Pause(char *error, size_t maxlen) override {
		return true;
	}
	bool Unpause(char *error, size_t maxlen) override {
		return true;
	}
	void AllPluginsLoaded() override {
	}
	//IMetamodListener stuff
	void OnVSPListening(IServerPluginCallbacks *iface) override {
	}

	//hooks
	bool Hook_LevelInit(const char *pMapName,
		char const *pMapEntities,
		char const *pOldLevel,
		char const *pLandmarkName,
		bool loadGame,
		bool background) const;
	void Hook_GameFrame(bool simulating);
	void Hook_LevelShutdown();

	void Hook_ClientDisconnect(edict_t *pEntity);

	void Hook_ClientPutInServer(edict_t *pEntity, char const *playername);

#if SOURCE_ENGINE >= SE_ORANGEBOX
	void Hook_ClientCommand(edict_t *pEntity, const CCommand &args);
#else
	void Hook_ClientCommand(edict_t *pEntity);
#endif
public:
	const char *GetAuthor() override {
		return "taiyungwang";
	}
	const char *GetName() override {
		return "HL2 GOAP Bot";
	}
	const char *GetDescription() override {
		return "A bot that supports DODS and HL2DM.";
	}
	const char *GetURL() override {
		return "https://github.com/taiyungwang/hl2_goap_bot";
	}
	const char *GetLicense() override {
		return "GPL2";
	}

	const char *GetVersion() override {
		return "1.3.2";
	}

	const char *GetDate() override {
		return __DATE__;
	}

	const char *GetLogTag() override {
		return "mybot";
	}

	void Hook_PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper);

private:
	std::shared_ptr<PluginAdaptor> adaptor;
};
