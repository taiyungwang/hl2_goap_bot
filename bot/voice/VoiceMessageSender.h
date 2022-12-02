#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <memory>

class VoiceMessage;

/**
 * Manages voice message sending for the bot.  It might make sense to move this into
 * the Bot class, but that would mean iserverplugin.h would be included.
 */

class VoiceMessageSender {
public:
	using TypeInfoRef = std::reference_wrapper<const std::type_info>;

	/**
	 * Associates a voice message string with the a VoiceMessage class.
	 * @param message String representation of the voice message as provided by the mod.
	 */
	template<typename T>
	void addMessage(const std::string &message) {
		messages[typeid(T)] = message;
	}

	bool sendMessage(const std::shared_ptr<VoiceMessage>& message);

	template<typename T>
	bool isMessage(const char* message) const {
		return messages.find(typeid(T)) != messages.end()
				&& messages.at(typeid(T)) == message;
	}

private:
	using SentMessage = std::tuple<float, std::shared_ptr<VoiceMessage>>;

	struct Hasher {
		std::size_t operator()(const TypeInfoRef &code) const {
			return code.get().hash_code();
		}
	};

	struct EqualTo {
		bool operator()(const TypeInfoRef &lhs, const TypeInfoRef &rhs) const {
			return lhs.get() == rhs.get();
		}
	};

	std::unordered_map<TypeInfoRef, std::string, Hasher, EqualTo> messages;

	std::list<SentMessage> sentMessages;
};

