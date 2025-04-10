#include <4dm.h>
#include "Config.h"
#include "Daytime.h"
#include "JSONData.h"
#include <fstream>

using namespace fdm;

inline static constexpr float DEFAULT_TIME = 0.45f;

inline static float server_time = DEFAULT_TIME;

void onDaytimeSync(WorldClient* world, Player* player, const nlohmann::json& data)
{
	try
	{
		StateGame::instanceObj.time = data["time"];
	}
	catch (const nlohmann::json::exception&)
	{
	}
}

void loadWorldTime(const stl::path& worldPath, float& time)
{
	if (!Config::daytime()) return;

	stl::string path = std::format("{}/info.json", worldPath.string());
	std::ifstream info{ path };
	if (!info.is_open()) return;

	nlohmann::json infoJson = nlohmann::json::parse(info);

	if (infoJson.is_object())
	{
		if (!infoJson.contains("time"))
			infoJson["time"] = DEFAULT_TIME;

		time = infoJson["time"];
	}

	info.close();
}
void saveWorldTime(const stl::path& worldPath, const float& time)
{
	if (!Config::daytime()) return;

	stl::string path = std::format("{}/info.json", worldPath.string());
	std::ifstream info{ path };
	if (!info.is_open()) return;

	nlohmann::json infoJson = nlohmann::json::parse(info);

	if (infoJson.is_object())
	{
		infoJson["time"] = time;
	}

	info.close();

	std::ofstream infoSave{ path };
	if (!infoSave.is_open()) return;

	infoSave << infoJson.dump();

	infoSave.close();
}

$hook(void, WorldServer, WorldServer, const stl::path& worldsPath, const stl::path& settingsPath, const stl::path& biomeInfoPath)
{
	original(self, worldsPath, settingsPath, biomeInfoPath);
	loadWorldTime(self->chunkLoader.worldPath, server_time);
}
$hook(void, WorldSingleplayer, WorldSingleplayer, const stl::path& worldPath, const stl::path& biomeInfoPath)
{
	loadWorldTime(worldPath, StateGame::instanceObj.time);
	original(self, worldPath, biomeInfoPath);
}

$hook(void, WorldServer, saveWorldData)
{
	saveWorldTime(self->chunkLoader.worldPath, server_time);
	return original(self);
}
$hook(void, WorldSingleplayer, destr_WorldSingleplayer)
{
	saveWorldTime(self->chunkLoader.worldPath, StateGame::instanceObj.time);
	original(self);
}
$hook(void, WorldServer, updateBackend, double dt)
{
	if (!Config::daytime()) return original(self, dt);

	server_time += 0.00004f;
	if (server_time > 1.0f)
	{
		server_time -= 1.0f;
		JSONData::broadcastPacket(self, S_DAYTIME_SYNC, { { "time", server_time } });
	}

	return original(self, dt);
}

$hook(void, StateIntro, init, StateManager& s)
{
	if (!Config::daytime()) return original(self, s);

	original(self, s);
	JSONData::SCaddPacketCallback(S_DAYTIME_SYNC, onDaytimeSync);
}
$hook(void, WorldClient, localPlayerInit, Player* player)
{
	if (!Config::daytime()) return original(self, player);

	StateGame::instanceObj.time = DEFAULT_TIME;
	original(self, player);
}

$hook(void, WorldServer, handleMessage, const Connection::InMessage& message, double dt)
{
	if (!Config::daytime()) return original(self, message, dt);

	uint16_t packet = message.getPacketType();
	if (packet == Packet::C_REJOIN || packet == Packet::C_JOIN || packet == Packet::C_RENDER_DIST)
	{
		JSONData::sendPacketClient(self, S_DAYTIME_SYNC, { { "time", server_time } }, message.getClient());
	}

	return original(self, message, dt);
}
