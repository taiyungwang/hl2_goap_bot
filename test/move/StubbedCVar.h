#pragma once

#include <convar.h>

/**
 * Used to test modules that require a ConVar.
 */
class StubbedCVar: public ICvar {
public:
	virtual ~StubbedCVar() {
	}
	// Here's where the app systems get to learn about each other
	bool Connect(CreateInterfaceFn factory) {
		return false;
	}
	void Disconnect() {
	}

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	void* QueryInterface(const char *pInterfaceName) {
		return nullptr;
	}

	// Init, shutdown
	InitReturnVal_t Init() {
		return INIT_FAILED;
	}
	void Shutdown() {
	}

	// Allocate a unique DLL identifier
	CVarDLLIdentifier_t AllocateDLLIdentifier() {
		return 0;
	}

	// Register, unregister commands
	void RegisterConCommand(ConCommandBase *pCommandBase) {
	}
	void UnregisterConCommand(ConCommandBase *pCommandBase) {
	}
	void UnregisterConCommands(CVarDLLIdentifier_t id) {
	}

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	const char* GetCommandLineValue(const char *pVariableName) {
		return nullptr;
	}

	// Try to find the cvar pointer by name
	ConCommandBase* FindCommandBase(const char *name) {
		return nullptr;
	}
	const ConCommandBase* FindCommandBase(const char *name) const {
		return nullptr;
	}
	ConVar* FindVar(const char *var_name) {
		return nullptr;
	}
	const ConVar* FindVar(const char *var_name) const {
		return nullptr;
	}
	ConCommand* FindCommand(const char *name) {
		return nullptr;
	}
	const ConCommand* FindCommand(const char *name) const {
		return nullptr;
	}

	// Get first ConCommandBase to allow iteration
	ConCommandBase* GetCommands(void) {
		return nullptr;
	}
	const ConCommandBase* GetCommands(void) const {
		return nullptr;
	}

	// Install a global change callback (to be called when any convar changes)
	void InstallGlobalChangeCallback(FnChangeCallback_t callback) {
	}
	void RemoveGlobalChangeCallback(FnChangeCallback_t callback) {
	}
	void CallGlobalChangeCallbacks(ConVar *var, const char *pOldString,
			float flOldValue) {
	}

	// Install a console printer
	void InstallConsoleDisplayFunc(IConsoleDisplayFunc *pDisplayFunc) {
	}
	void RemoveConsoleDisplayFunc(IConsoleDisplayFunc *pDisplayFunc) {
	}
	void ConsoleColorPrintf(const Color &clr,
			PRINTF_FORMAT_STRING const char *pFormat, ...) const
					FMTFUNCTION( 3, 4 ) {
	}
	void ConsolePrintf( PRINTF_FORMAT_STRING const char *pFormat, ...) const
			FMTFUNCTION( 2, 3 ) {
	}
	void ConsoleDPrintf( PRINTF_FORMAT_STRING const char *pFormat, ...) const
			FMTFUNCTION( 2, 3 ) {
	}

	// Reverts cvars which contain a specific flag
	void RevertFlaggedConVars(int nFlag) {
	}

	// Method allowing the engine ICvarQuery interface to take over
	// A little hacky, owing to the fact the engine is loaded
	// well after ICVar, so we can't use the standard connect pattern
	void InstallCVarQuery(ICvarQuery *pQuery) {
	}

	bool IsMaterialThreadSetAllowed() const {
		return false;
	}
	void QueueMaterialThreadSetValue(ConVar *pConVar, const char *pValue) {
	}
	void QueueMaterialThreadSetValue(ConVar *pConVar, int nValue) {
	}
	void QueueMaterialThreadSetValue(ConVar *pConVar, float flValue) {
	}
	bool HasQueuedMaterialThreadConVarSets() const {
		return false;
	}
	int ProcessQueuedMaterialThreadConVarSets() {
		return 0;
	}
protected:
	virtual ICVarIteratorInternal* FactoryInternalIterator(void) {
		return nullptr;
	}
};
