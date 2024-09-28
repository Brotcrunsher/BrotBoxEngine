#include <future>
#include <vector>
#include <mutex>

#include "../BBE/String.h"
#include "../BBE/List.h"
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

	public:
		bbe::String key = "";
        bbe::String model = "gpt-4o";
        bbe::String role = "You are a helpful assistant.";
        std::vector<nlohmann::json> history; // Not a bbe::List, because nlohmann works much simpler with std::vector
		int32_t totalTokens = 0;
		mutable std::mutex mutex;

		ChatGPTComm() = default;
		ChatGPTComm(const bbe::String& key);

		bool isKeySet() const;

        ChatGPTQueryResponse query(const bbe::String& msg);
        std::future<ChatGPTQueryResponse> queryAsync(const bbe::String& msg);

        void purgeMemory();

        bbe::List<bbe::String> getAvailableModels() const;
	};
}
