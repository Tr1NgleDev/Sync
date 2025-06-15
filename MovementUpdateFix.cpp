#include <4dm.h>
#include "Config.h"

using namespace fdm;

// already implemented by mashpoe

// server-side fix
$hook(void, WorldServer, handleMessage, const Connection::InMessage& message, double dt)
{
	if (!Config::movementUpdateFix()) return original(self, message, dt);

	if (!self->players.contains(message.getClient()))
		return original(self, message, dt);

	auto& playerInfo = self->players.at(message.getClient());
	Player* player = playerInfo.player.get();
	if (message.getPacketType() == Packet::C_MOVEMENT_UPDATE)
	{
		try
		{
			nlohmann::json j = nlohmann::json::parse(message.getStrData());

			Chunk* c = self->getChunkFromCoords(player->pos.x, player->pos.z, player->pos.w);

			player->applyMovementUpdate(j);

			if (c != nullptr)
			{
				nlohmann::json clientUpdate{};
				clientUpdate["entity"] = (std::string)stl::uuid::to_string(player->EntityPlayerID);
				clientUpdate["value"] = j;
				stl::string msgData = clientUpdate.dump();
				Connection::OutMessage message{ Packet::S_PLAYER_MOVEMENT_UPDATE, msgData };
				self->sendMessageOtherPlayers(message, c, &playerInfo, false);
			}

			if (glfwGetTime() - player->lastChunkUpdateTime < 0.2) // also lowered this from 0.5 to 0.2
			{
				return;
			}

			player->lastChunkUpdateTime = glfwGetTime();

			bool chunkUpdate = false;

			float dAngle = glm::acos(glm::clamp(glm::dot(player->over, player->lastChunkOver), -1.0f, 1.0f));
			
			if (dAngle > 0.001f)
			{
				chunkUpdate = true;
			}
			else
			{
				glm::vec4 diff = player->pos - player->lastChunkPos;
				float distSqr = glm::dot(diff, diff);

				if (distSqr >= 8.0f * 8.0f)
				{
					chunkUpdate = true;
				}
			}

			if (chunkUpdate)
			{
				self->updateChunks(&playerInfo);
			}
		}
		catch (const nlohmann::json::exception& e)
		{
		}

		return;
	}
	return original(self, message, dt);
}
