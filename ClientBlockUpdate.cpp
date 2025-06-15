#include <4dm.h>
#include "Config.h"

using namespace fdm;

struct BlockUpdateData
{
	double time = 0;
	uint8_t oldID = 0;
	uint8_t newID = 0;
	bool mining = false;
};

inline static std::unordered_map<glm::i64vec4, BlockUpdateData> updateData{};
inline static double measurementStartTime = 0.0;
inline static bool measurementStarted = false;
inline static double avgPingSum = 0.0;
inline static uint64_t avgPingCounter = 0;
inline static bool clientUpdate = false;

$hook(bool, WorldManager, setBlockUpdate, const glm::ivec4& block, uint8_t value)
{
	if (!Config::clientBlockUpdate()) return original(self, block, value);

	if (clientUpdate && self->getType() == World::TYPE_CLIENT)
	{
		auto& update = updateData[block];
		update.oldID = self->getBlock(block);
		update.newID = value;
		update.time = glfwGetTime();
	}
	return original(self, block, value);
}

$hook(bool, WorldClient, addEntityToChunk, std::unique_ptr<Entity>& entity, Chunk* c)
{
	if (!Config::clientBlockUpdate()) return original(self, entity, c);

	if (!clientUpdate)
		return original(self, entity, c);
	delete entity.get();
	entity.release();
	return false;
}

$hook(void, WorldClient, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data)
{
	if (!Config::clientBlockUpdate()) return original(self, player, eventType, eventValue, data);

	auto& item = player->getSelectedHotbarSlot();
	if (eventType == Packet::C_ITEM_ACTION
		&& eventValue == GLFW_MOUSE_BUTTON_RIGHT
		&& player->rightClickTimerReady()
		&& player->canPlace
		&& player->targetingBlock
		&& item
		&& item->count >= 1
		&& 0 == strcmp(typeid(*item.get()).name(), "class ItemBlock")
		&& !updateData.contains(player->targetPlaceBlock))
	{
		/*int legs = floorf(player->pos.y);
		int head = floorf(player->pos.y + Player::HEIGHT);
		if (player->targetPlaceBlock.x == player->currentBlock.x &&
			player->targetPlaceBlock.z == player->currentBlock.z &&
			player->targetPlaceBlock.w == player->currentBlock.w &&
			(player->targetPlaceBlock.y == legs || player->targetPlaceBlock.y == head))
			return original(self, player, eventType, eventValue, data);
			*/

		auto* itemBlock = (ItemBlock*)item.get();
		uint8_t oldID = self->getBlock(player->targetPlaceBlock);
		uint8_t newID = itemBlock->blockID;
		if ((oldID != newID) &&
			(oldID == BlockInfo::AIR || oldID == BlockInfo::LAVA || newID == BlockInfo::AIR))
		{
			//auto& update = updateData[player->targetPlaceBlock];
			//update.newID = newID;
			//update.oldID = oldID;
			//update.time = glfwGetTime();
			//self->setBlockUpdate(player->targetPlaceBlock, itemBlock->blockID);
			player->rightClickActionTime = glfwGetTime();
			clientUpdate = true;
			if (itemBlock->action(self, player, eventValue))
			{
				measurementStartTime = glfwGetTime();
				measurementStarted = true;
			}
			clientUpdate = false;
		}
		else
			return;
	}
	else if (eventType == Packet::C_BLOCK_BREAK_FINISH && player->targetingBlock && player->targetDamage >= 1.0 - 0.001)
	{
		auto targetBlock = player->targetBlock;
		clientUpdate = true;
		if (player->breakBlock(self))
		{
			player->targetBlock = targetBlock;
			player->targetingBlock = true;
			player->targetDamage = 1;
			{
				measurementStartTime = glfwGetTime();
				measurementStarted = true;
			}
			auto& update = updateData[player->targetBlock];
			update.mining = true;
		}
		clientUpdate = false;
	}

	return original(self, player, eventType, eventValue, data);
}

$hook(void, WorldClient, destr_WorldClient)
{
	if (!Config::clientBlockUpdate()) return original(self);

	updateData.clear();
	clientUpdate = false;
	avgPingSum = 0;
	avgPingCounter = 0;
	measurementStartTime = 0.0;
	measurementStarted = false;

	original(self);
}

$hook(void, WorldClient, updateLocal, StateManager& s, Player* player, double dt)
{
	if (!Config::clientBlockUpdate()) return original(self, s, player, dt);

	clientUpdate = false;
	double curTime = glfwGetTime();
	double timeout = 1.0;
	if (avgPingCounter > 0)
	{
		timeout = glm::max(avgPingSum / avgPingCounter + 0.5, timeout);
	}
	for (std::unordered_map<glm::i64vec4, BlockUpdateData>::iterator it = updateData.begin(); it != updateData.end();)
	{
		if (curTime - it->second.time >= timeout)
		{
			self->setBlockUpdate(it->first, it->second.oldID);
			it = updateData.erase(it);
			continue;
		}
		if (it->second.mining)
		{
			auto targetBlock = player->targetBlock;
			auto targetingBlock = player->targetingBlock;
			auto targetDamage = player->targetDamage;
			player->targetBlock = it->first;
			player->targetingBlock = true;
			player->targetDamage = 1;
			fdmHooks46::localPlayerEventH::original(self, player, Packet::C_BLOCK_BREAK_FINISH, 0, 0);
			player->targetBlock = targetBlock;
			player->targetingBlock = targetingBlock;
			player->targetDamage = targetDamage;
		}
		++it;
	}

	return original(self, s, player, dt);
}

$hook(bool, WorldClient, handleWorldMessage, const Connection::InMessage& message, Player* player)
{
	if (!Config::clientBlockUpdate()) return original(self, message, player);

	if (message.getPacketType() == Packet::S_BLOCK_UPDATE)
	{
		if (measurementStarted)
		{
			if (avgPingCounter > 32)
			{
				avgPingSum = avgPingSum / avgPingCounter;
				avgPingCounter = 1;
			}
			avgPingSum += glfwGetTime() - measurementStartTime;
			++avgPingCounter;
			measurementStarted = false;
		}
		try
		{
			nlohmann::json j = nlohmann::json::parse(message.getStrData());
			for (auto& i : j.items())
			{
				glm::ivec4 pos = m4::vec4FromJson<int>(nlohmann::json::parse(i.key()));
				//uint8_t blockID = i.value().get<int>();
				if (updateData.contains(pos))
					updateData.erase(pos);
			}
		}
		catch (const nlohmann::json::exception&) { }
	}

	return original(self, message, player);
}

$hook(void, ItemBlock, postAction, World* world, Player* player, int action)
{
	if (!Config::clientBlockUpdate()) return original(self, world, player, action);

	double rightClickActionTime = player->rightClickActionTime;
	original(self, world, player, action);
	if (action == GLFW_MOUSE_BUTTON_RIGHT && world->getType() == World::TYPE_CLIENT)
	{
		player->rightClickActionTime = rightClickActionTime;
	}
}
