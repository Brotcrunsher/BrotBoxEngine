#pragma once
#include <future>
#include <vector>
#include <mutex>

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/Sound.h"
#include "nlohmann/json.hpp"

namespace bbe
{
	struct ChatGPTQueryResponse
	{
		bbe::String message;
		int32_t inputTokens = 0;
		int32_t outputTokens = 0;
		int32_t totalTokens = 0;
	};

	class ChatGPTComm
	{
	private:
		std::string sendRequest(const std::string& url, const std::string& jsonInput) const;
		bbe::List<char> sendRequestBinary(const std::string& url, const std::string& jsonInput) const;

	public:
		bbe::String key = "";
		bbe::String model = "gpt-4";
		bbe::String ttsModel = "tts-1-hd";
		bbe::String role = "You are a helpful assistant.";
		bbe::String voice = "alloy"; // Possible values currently: "alloy", "echo", "fable", "onyx", "nova", or "shimmer"
		std::vector<nlohmann::json> history; // Not a bbe::List, because nlohmann works much simpler with std::vector
		int32_t totalTokens = 0;
		mutable std::mutex mutex;

		ChatGPTComm() = default;
		ChatGPTComm(const bbe::String& key);

		bool isKeySet() const;

		ChatGPTQueryResponse query(const bbe::String& msg);
		std::future<ChatGPTQueryResponse> queryAsync(const bbe::String& msg);

		bbe::Sound synthesizeSpeech(const bbe::String& text);
		std::future<bbe::Sound> synthesizeSpeechAsync(const bbe::String& text);

		void purgeMemory();

		bbe::List<bbe::String> getAvailableModels() const;
	};
}
