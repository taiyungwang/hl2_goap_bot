class EventInfo {
public:
	virtual ~EventInfo() {
	}

	virtual float getFloat(const char *keyName = 0, float defaultValue = 0) = 0;

	virtual int getInt(const char *keyName = 0, int defaultValue = 0) = 0;

	virtual const char *getString(const char *keyName = 0,
			const char *defaultValue = 0) = 0;

	virtual const char *getName() = 0;

	virtual void setInt(const char *keyName, int value) = 0;
};

template<typename T>
class EventInfoWrapper: public EventInfo {
public:

	EventInfoWrapper(T *pEvent) :
			info(pEvent) {
	}

	float getFloat(const char *keyName = 0, float defaultValue = 0) {
		return info->GetFloat(keyName, defaultValue);
	}

	int getInt(const char *keyName = 0, int defaultValue = 0) {
		return info->GetInt(keyName, defaultValue);
	}

	void setInt(const char *keyName, int value) {
		info->SetInt(keyName, value);
	}

	const char *getString(const char *keyName = 0,
			const char *defaultValue = 0) {
		return info->GetString(keyName, defaultValue);
	}

	const char *getName() {
		return info->GetName();
	}
private:
	T* info;
};
