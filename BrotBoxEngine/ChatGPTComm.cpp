#include "BBE/ChatGPTComm.h"
#include "BBE/Logging.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/Async.h"

std::string bbe::ChatGPTComm::sendRequest(const std::string& url, const std::string& jsonInput) const {
    auto response = bbe::simpleUrlRequest::urlRequest(url.c_str(), { "Authorization: Bearer " + key, "Content-Type: application/json" }, jsonInput.c_str());

    return response.dataContainer.getRaw();
}

bbe::ChatGPTComm::ChatGPTComm(const bbe::String& key) : key(key)
{
}

bool bbe::ChatGPTComm::isKeySet() const
{
    std::lock_guard _(mutex);
    return key.isEmpty() == false;
}

bbe::ChatGPTQueryResponse bbe::ChatGPTComm::query(const bbe::String& msg)
{
    std::lock_guard _(mutex);
    // Add user's message to the conversation
    history.push_back({ {"role", "user"}, {"content", msg.getRaw()}});

    std::vector<nlohmann::json> sendMessages = {
        {{"role", "system"}, {"content", role.getRaw()}}
    };
    for (size_t i = 0; i < history.size(); i++)
    {
        sendMessages.push_back(history[i]);
    }

    // Prepare the JSON data for the request
    nlohmann::json jsonData = {
        {"model", model.getRaw()},
        {"messages", sendMessages}
    };

    // Send the request
    std::string response = sendRequest("https://api.openai.com/v1/chat/completions", jsonData.dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore));

    // Parse and display the response
    try {
        ChatGPTQueryResponse retVal;
        auto jsonResponse = nlohmann::json::parse(response);
        retVal.message = jsonResponse["choices"][0]["message"]["content"].get<std::string>().c_str();

        retVal.inputTokens = jsonResponse["usage"]["prompt_tokens"];
        retVal.outputTokens = jsonResponse["usage"]["completion_tokens"];
        retVal.totalTokens = jsonResponse["usage"]["total_tokens"];
        totalTokens += retVal.totalTokens;

        // Add assistant's message to the conversation
        history.push_back({ {"role", "assistant"}, {"content", retVal.message.getRaw()}});
        return retVal;
    }
    catch (const std::exception& e) {
        BBELOGLN("Error parsing API response: " << e.what());
    }

    bbe::Crash(bbe::Error::IllegalState, "Error parsing API response");
}

std::future<bbe::ChatGPTQueryResponse> bbe::ChatGPTComm::queryAsync(const bbe::String& msg)
{
    return bbe::async(&bbe::ChatGPTComm::query, this, msg);
}

void bbe::ChatGPTComm::purgeMemory()
{
    std::lock_guard _(mutex);
    history.clear();
}

bbe::List<bbe::String> bbe::ChatGPTComm::getAvailableModels() const
{
    std::lock_guard _(mutex);
    std::string url = "https://api.openai.com/v1/models";
    std::string response = sendRequest(url, "");

    bbe::List<bbe::String> models;
    try {
        auto jsonResponse = nlohmann::json::parse(response);
        for (const auto& model : jsonResponse["data"]) {
            models.add(model["id"].get<std::string>().c_str());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
    }

    if (models.isEmpty()) {
        bbe::Crash(bbe::Error::IllegalState, "No models available or failed to retrieve models.");
    }

    return models;
}
