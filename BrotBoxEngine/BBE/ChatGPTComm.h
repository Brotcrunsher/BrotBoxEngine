#pragma once
#ifdef WIN32
#include <future>
#include <vector>
#include <mutex>

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/Sound.h"
#include "../BBE/Image.h"
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

	struct ChatGPTCreateImageResponse
	{
		bbe::Image image;
		bbe::String url;
	};

	class ChatGPTComm
	{
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

		bbe::String transcribe(const bbe::Sound& sound);
		std::future<bbe::String> transcribeAsync(const bbe::Sound& sound);

		ChatGPTCreateImageResponse createImage(const bbe::String& prompt, const bbe::Vector2i& size);
		std::future<ChatGPTCreateImageResponse> createImageAsync(const bbe::String& prompt, const bbe::Vector2i& size);

		// TODO: base 64 encoded image data instead of urls
		bbe::String describeImage(const nlohmann::json& requestJson);
		bbe::String describeImage(const bbe::String& url);
		std::future<bbe::String> describeImageAsync(const nlohmann::json& requestJson);
		std::future<bbe::String> describeImageAsync(const bbe::String& url);

		void purgeMemory();

		bbe::List<bbe::String> getAvailableModels() const;
	};
}
#endif
