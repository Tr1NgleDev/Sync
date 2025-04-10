#include <4dm.h>
#include "Config.h"

using namespace fdm;

// custom member setup (works for most classes but not all because some lack a destructor or a constructor or both)
struct _Player
{
	bool isBreakingBlock = false;
	_Player(Player* self) {}
	~_Player() {}
};
inline static std::unordered_map<Player*, _Player> _Player_data{};
inline __forceinline static _Player* playerData(Player* player)
{
	if (_Player_data.contains(player))
		return &_Player_data.at(player);
	return nullptr;
}
$hook(void, Player, Player)
{
	if (!Config::instaMiningFix()) return original(self);

	Player* a = self;
	_Player_data.try_emplace(a, a);

	original(self);
}
$hook(void, Player, destr_Player)
{
	if (!Config::instaMiningFix()) return original(self);

	if (_Player_data.contains(self))
		_Player_data.erase(self);

	original(self);
}

$hook(void, WorldServer, updateBackend, double dt)
{
	if (!Config::instaMiningFix()) return original(self, dt);

	using PlayerInfo = WorldServer::PlayerInfo;

	for (auto& entry : self->players)
	{
		PlayerInfo& playerInfo = entry.second;
		Player* player = playerInfo.player.get();
		auto* playerD = playerData(player);
		if (playerD &&
			playerD->isBreakingBlock)
		{
			player->hitTargetBlock(self);
		}
	}

	return original(self, dt);
}

$hook(void, WorldServer, handleMessage, const Connection::InMessage& message, double dt)
{
	if (!Config::instaMiningFix()) return original(self, message, dt);

	using PlayerInfo = WorldServer::PlayerInfo;

	if (!self->players.contains(message.getClient()))
		return original(self, message, dt);

	PlayerInfo* playerInfo = &self->players.at(message.getClient());
	Player* player = playerInfo->player.get();
	if (!player)
		return original(self, message, dt);
	auto* playerD = playerData(player);
	if (!playerD)
		return original(self, message, dt);

	switch (message.getPacketType())
	{
	case Packet::C_BLOCK_BREAK_START:
	{
		// guard against invalid packet data
		if (message.getMsgSize() != sizeof(glm::ivec4))
		{
			break;
		}
		const glm::ivec4* targetBlock = (const glm::ivec4*)message.getMsgData();

		player->targetBlock = *targetBlock;
		player->targetingBlock = true;
		playerD->isBreakingBlock = true;
		player->targetDamage = 0.05; // just in case

		Chunk* c = self->getChunkFromCoords(targetBlock->x, targetBlock->z, targetBlock->w);
		if (c == nullptr)
		{
			break;
		}
		nlohmann::json j;
		j["entity"] = stl::uuid::to_string(player->EntityPlayerID);
		j["targetBlock"] = m4::vec4ToJson(*targetBlock);
		Connection::OutMessage breakMessage{ Packet::S_PLAYER_BLOCK_BREAK_START, j.dump() };
		self->sendMessageOtherPlayers(breakMessage, c, playerInfo, true);

		break;
	}
	case Packet::C_BLOCK_BREAK_CANCEL:
	{
		playerD->isBreakingBlock = false;
		if (player->targetDamage != 0.0f)
		{
			player->targetDamage = 0.0f;
			self->localPlayerEvent(player, Packet::C_BLOCK_BREAK_CANCEL, 0, 0);
		}

		Chunk* c = self->getChunkFromCoords(player->targetBlock.x, player->targetBlock.z, player->targetBlock.w);
		if (c == nullptr)
		{
			break;
		}
		nlohmann::json j;
		j["entity"] = stl::uuid::to_string(player->EntityPlayerID);
		Connection::OutMessage breakMessage{ Packet::S_PLAYER_BLOCK_BREAK_STOP, j.dump() };
		self->sendMessageOtherPlayers(breakMessage, c, playerInfo, true);

		break;
	}
	case Packet::C_BLOCK_BREAK_FINISH:
	{
		// guard against invalid packet data
		if (message.getMsgSize() != sizeof(glm::ivec4))
		{
			break;
		}
		const glm::ivec4* targetBlock = (const glm::ivec4*)message.getMsgData();

		if (player->targetDamage < 0.9f)
		{
			break;
		}
		playerD->isBreakingBlock = false;

		Chunk* c = self->getChunkFromCoords(targetBlock->x, targetBlock->z, targetBlock->w);
		if (c == nullptr)
		{
			break;
		}

		uint8_t blockID = self->getBlock(*targetBlock);

		if (c != nullptr && player->targetBlock == *targetBlock && player->breakBlock(self))
		{
			//Console::print<0x00FFFF>(BlockInfo::getBlockName(blockID), " block broken!");
		}

		nlohmann::json j;
		j["entity"] = stl::uuid::to_string(player->EntityPlayerID);
		Connection::OutMessage breakMessage{ Packet::S_PLAYER_BLOCK_BREAK_STOP, j.dump() };
		self->sendMessageOtherPlayers(breakMessage, c, playerInfo, true);

		break;
	}
	default:
		return original(self, message, dt);
	}
}
