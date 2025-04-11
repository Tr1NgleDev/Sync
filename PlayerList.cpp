#include <4dm.h>
#include "Config.h"
#include "JSONData.h"
#include "PlayerList.h"
#include "ilerp.h"
#include "4DKeyBinds.h"

using namespace fdm;

std::vector<std::pair<fdm::stl::uuid, fdm::stl::string>> playerList{};

// client-side
void onPlayerListReceive(WorldClient* world, Player* player, const nlohmann::json& data)
{
	try
	{
		playerList.clear();
		playerList.reserve(data.size());
		for (auto& player : data)
		{
			playerList.emplace_back(stl::uuid()(player["entity"].get<std::string>()), stl::string(player["name"].get<std::string>()));
		}
	}
	catch (const nlohmann::json::exception&)
	{
	}
	catch (const std::runtime_error&)
	{
	}
}

// server-side
void onPlayerListRequest(WorldServer* world, double dt, const nlohmann::json& data, uint32_t client)
{
	nlohmann::json j = nlohmann::json::array();

	for (auto& player : world->players)
	{
		j.push_back(nlohmann::json{ { "entity", stl::uuid::to_string(player.second.player->EntityPlayerID) }, { "name", player.second.displayName }});
	}

	JSONData::sendPacketClient(world, S_PLAYER_LIST, j, client);
}

// server-side
void addCallback()
{
	if (!Config::playerList()) return;

	JSONData::CSaddPacketCallback(C_PLAYER_LIST_REQUEST, onPlayerListRequest);
}
$hook(void, WorldServer, WorldServer, const stl::path& worldsPath, const stl::path& settingsPath, const stl::path& biomeInfoPath)
{		
	addCallback();
	original(self, worldsPath, settingsPath, biomeInfoPath);
}

extern "C" __declspec(dllexport) void PlayerList_requestPlayerList(fdm::WorldClient* world)
{
	if (!Config::playerList()) return;

	JSONData::sendPacketServer(world, C_PLAYER_LIST_REQUEST, {});
}
extern "C" __declspec(dllexport) const std::vector<std::pair<fdm::stl::uuid, fdm::stl::string>>& PlayerList_getPlayerList()
{
	if (!Config::playerList()) return {};

	return playerList;
}

void clearPlayerList()
{
	if (!Config::playerList()) return;

	playerList.clear();
}
$hook(void, WorldClient, destr_WorldClient)
{
	clearPlayerList();
	original(self);
}

$hook(void, WorldServer, handleMessage, const Connection::InMessage& message, double dt)
{
	if (!Config::playerList()) return original(self, message, dt);

	uint16_t packet = message.getPacketType();
	if (packet == Packet::C_REJOIN || packet == Packet::C_JOIN)
	{
		nlohmann::json j = nlohmann::json::array();

		for (auto& player : self->players)
		{
			j.push_back(nlohmann::json{ { "entity", stl::uuid::to_string(player.second.player->EntityPlayerID) }, { "name", player.second.displayName } });
		}

		JSONData::broadcastPacket(self, S_PLAYER_LIST, j);
	}

	return original(self, message, dt);
}

// custom ui

inline static bool playerListOpen = false;

inline static constexpr float easeOutExpo(float x)
{
	return x == 1.0f ? 1.0f : 1.0f - pow(2.0f, -10.0f * x);
}

$hook(void, StateGame, windowResize, StateManager& s, GLsizei width, GLsizei height)
{
	if (!Config::playerList()) return original(self, s, width, height);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, 0, &projection2D[0][0]);

	return original(self, s, width, height);
}

$hook(void, Player, renderHud, GLFWwindow* window)
{
	if (!Config::playerList()) return original(self, window);

	static double playerListTime = -0.1;
	double curTime = glfwGetTime();
	static double dt = 0.01;
	static double lastTime = curTime;
	dt = curTime - lastTime;
	lastTime = curTime;

	if (StateGame::instanceObj.world->getType() != World::TYPE_CLIENT)
	{
		playerListOpen = false;
	}

	if (playerListOpen && playerListTime < 1.0)
	{
		playerListTime += dt / 0.5f;
	}

	glDepthMask(0);
	glDisable(GL_DEPTH_TEST);

	static int w = 0;
	static int h = 0;
	static int lastW = 0;
	static int lastH = 0;

	QuadRenderer& qr = StateGame::instanceObj.qr;
	FontRenderer& font = StateGame::instanceObj.font;

	static auto text = [&](const std::string& text, const glm::vec4& color, const glm::ivec2& pos)
	{
		font.fontSize = 1;
		glm::ivec2 posA
		{
			pos.x,
			(int)lerp(-100.0f, pos.y * (10 * font.fontSize) + 2 + 4 * font.fontSize, easeOutExpo(glm::clamp(playerListTime / 1.0, 0.0, 1.0)))
		};
		font.centered = true;
		font.setText(text);
		// shadow
		font.color = { 0,0,0,0.2 };
		font.pos = posA + glm::ivec2{ 1, 1 };
		font.updateModel();
		font.render();
		// text
		font.color = color;
		font.pos = posA;
		font.updateModel();
		font.render();

		w = glm::max(w, font.fontSize * 8 * (int)text.size());
		h = glm::max(h, posA.y + font.fontSize * (font.charSize.y / 2));
	};

	w = 0;
	h = 0;

	if (playerListTime >= 0.0)
	{
		int wWidth, wHeight;
		glfwGetWindowSize(window, &wWidth, &wHeight);

		qr.setQuadRendererMode(QuadRenderer::MODE_FILL);
		qr.setColor(0, 0, 0, 0.5);
		qr.setPos(wWidth / 2 - lastW / 2 - 2, 0, lastW + 5, lastH + 5);
		qr.render();

		int line = -1;

		if (playerList.empty())
		{
			++line;
			text("This server doesn't support Player List Sync :( (or you just didn't get the list yet)", glm::vec4{ 1 }, { wWidth / 2, line });
		}
		else
		{
			int maxW = 0;
			for (auto& player : playerList)
			{
				maxW = glm::max(maxW, ((int)player.second.size() * font.charSize.x + 16) / 2);
			}
			for (auto& player : playerList)
			{
				++line;
				const int rows = 12;
				const int cols = playerList.size() / rows;
				const int row = line % rows;
				const int col = line / rows;
				int x = wWidth / 2 - cols * maxW + col * (maxW * 2);
				text(player.second, glm::vec4{ 1 }, { x, row });
			}
		}

		if (!playerListOpen)
			playerListTime -= dt / 0.5f;
	}

	lastW = w;
	lastH = h;

	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);

	return original(self, window);
}

void showPlayerList(GLFWwindow* window, int action, int mods)
{
	if (StateGame::instanceObj.world->getType() != World::TYPE_CLIENT)
	{
		playerListOpen = false;
		return;
	}

	if (action == GLFW_REPEAT)
	{
		return;
	}

	playerListOpen = action == GLFW_PRESS;

	PlayerList_requestPlayerList((WorldClient*)StateGame::instanceObj.world.get());
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, char mods)
{
	if (!Config::playerList()) return original(self, window, world, key, scancode, action, mods);

	if (!KeyBinds::isLoaded() && key == GLFW_KEY_APOSTROPHE)
	{
		showPlayerList(window, action, mods);
	}

	return original(self, window, world, key, scancode, action, mods);
}

$hook(void, StateIntro, init, StateManager& s)
{
	if (!Config::playerList()) return original(self, s);

	KeyBinds::addBind("Sync", "Show Player List", glfw::Keys::Y, KeyBindsScope::PLAYER, showPlayerList);

	original(self, s);

	JSONData::SCaddPacketCallback(S_PLAYER_LIST, onPlayerListReceive);
}
