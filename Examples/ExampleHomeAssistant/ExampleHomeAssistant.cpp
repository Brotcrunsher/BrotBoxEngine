#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <iostream>

class MyGame : public bbe::Game
{
	bbe::String accessToken = "";
	bbe::String input;
    bbe::String reply;

    std::map<std::string, bool> rooms;

	virtual void onStart() override
	{
        rooms["Living room"] = false;
        rooms["Dining room"] = false;
        rooms["Kitchen"] = false;
        rooms["Bathroom"] = false;
	}

    const bbe::Image* getLampState(const char* room)
    {
        return rooms[room] ? assetStore::LampOn() : assetStore::LampOff();
    }

    void ToggleLight(const char* room, bool value)
    {
        rooms[room] = value;
    }

    void renderRoom(bbe::PrimitiveBrush2D& brush, const char* room, float x, float y)
    {
        brush.drawImage(x, y, 189.f / 2.f, 338.f / 2.f, *getLampState(room));
        brush.fillText(x + 189.f / 4.f, y, room, 40, bbe::Anchor::BOTTOM_CENTER);
    }

	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		ImGuiViewport fullViewport = *ImGui::GetMainViewport();
		fullViewport.WorkSize.x *= 0.5f;
		ImGui::SetNextWindowPos(fullViewport.WorkPos);
		ImGui::SetNextWindowSize(fullViewport.WorkSize);
		ImGui::Begin("Window", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			ImGui::bbe::InputText("Access Token", accessToken, ImGuiInputTextFlags_Password);
			if (ImGui::BeginTabBar("MainWindowTabs"))
			{
                if (ImGui::BeginTabItem("Demo1"))
                {
                    bool send = false;
                    send |= ImGui::bbe::InputText("Input", input, ImGuiInputTextFlags_EnterReturnsTrue);
                    send |= ImGui::Button("Send");
                    ImGui::SameLine();
                    if (ImGui::Button("Answer To Clipboard"))
                    {
                        setClipboard(reply);
                    }

                    if (send)
                    {
                        auto body = R"({
    "model": "openai/gpt-oss-120b:cerebras",
    "messages": [
        {
            "role": "system",
            "content": "Be a helpful bot. Answer in ASCII Characters only. No markdown."
        },
        {
            "role": "user",
            "content": ")" + input + R"("
        }
    ]
})";
                        auto request = bbe::simpleUrlRequest::urlRequest(
                            "https://router.huggingface.co/v1/chat/completions",
                            { "Authorization: Bearer " + accessToken, "Content-Type: application/json" },
                            body);
                        reply = nlohmann::json::parse(request.dataContainer.getRaw()).dump(1).c_str();
                        reply = reply.hardBreakEvery(89);
                    }
                    ImGui::Text("%s", reply.getRaw());

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Demo2"))
                {
                    bool send = false;
                    send |= ImGui::bbe::InputText("Input", input, ImGuiInputTextFlags_EnterReturnsTrue);
                    send |= ImGui::Button("Send");
                    ImGui::SameLine();
                    if (ImGui::Button("Answer To Clipboard"))
                    {
                        setClipboard(reply);
                    }

                    if (send)
                    {
                        auto body = R"({
    "model": "openai/gpt-oss-120b:cerebras",
    "messages": [
        {
            "role": "system",
            "content": "Be a useful home assistant."
        },
        {
            "role": "user",
            "content": ")" + input + R"("
        }
    ],
    "tool_choice": "auto",
    "tools": [
        {
            "type": "function",
            "function": {
                "name": "toggle_light",
                "description": "Turns a light on or off.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "device_id": {
                            "type": "string",
                            "enum": [
                                "Living room",
                                "Dining room",
                                "Kitchen",
                                "Bathroom"
                            ]
                        },
                        "state": {
                            "type": "string",
                            "enum": [
                                "on",
                                "off"
                            ]
                        }
                    },
                    "required": [
                        "device_id",
                        "state"
                    ],
                    "additionalProperties": false
                }
            }
        }
    ]
})";
                        auto request = bbe::simpleUrlRequest::urlRequest(
                            "https://router.huggingface.co/v1/chat/completions",
                            { "Authorization: Bearer " + accessToken, "Content-Type: application/json" },
                            body);
                        auto replyJson = nlohmann::json::parse(request.dataContainer.getRaw());
                        auto nameLocation = nlohmann::json::json_pointer("/choices/0/message/tool_calls/0/function/name");
                        if (replyJson.contains(nameLocation) && replyJson[nameLocation] == "toggle_light")
                        {
                            auto argumentsJson = nlohmann::json::parse(replyJson[nlohmann::json::json_pointer("/choices/0/message/tool_calls/0/function/arguments")].get<std::string>());
                            ToggleLight(argumentsJson["device_id"].get<std::string>().c_str(), argumentsJson["state"] == "on");
                        }
                        reply = nlohmann::json::parse(request.dataContainer.getRaw()).dump(1).c_str();
                        reply = reply.hardBreakEvery(89);
                    }
                    ImGui::Text("%s", reply.getRaw());



                    renderRoom(brush, "Living room", 640 + 320 - 95 - 150, 100);
                    renderRoom(brush, "Dining room", 640 + 320 - 95 + 150, 100);
                    renderRoom(brush, "Kitchen", 640 + 320 - 95 - 150, 400);
                    renderRoom(brush, "Bathroom", 640 + 320 - 95 + 150, 400);

                    ImGui::EndTabItem();
                }
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
        
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame* mg = new MyGame();
	mg->start(1280, 720, "Home Assistant!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

