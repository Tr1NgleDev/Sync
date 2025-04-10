#include <4dm.h>
#include "Config.h"
#include "JSONData.h"
#include "Health.h"

using namespace fdm;

std::unordered_map<fdm::stl::string, EntityInfo> entityInfo{};

void init4DMEntities();

// server-side
float getEntityHealth(Entity* entity)
{
	stl::string name = entity->getName();
	if (entityInfo.contains(name))
		return entityInfo[name].getHealth(entity);
	return 0;
}

// server-side
void sendPacketInRegion(WorldServer* world, const fdm::stl::string& packet, const nlohmann::json& data, const glm::vec4& origin)
{
	for (int x = -1; x <= 1; ++x)
		for (int z = -1; z <= 1; ++z)
			for (int w = -1; w <= 1; ++w)
			{
				Chunk* chunk = world->getChunkFromCoords(origin.x + 8 * x, origin.z + 8 * z, origin.w + 8 * w);

				if (!chunk) continue;

				for (auto& entity : chunk->entities)
				{
					if (entity->getName() != "Player") continue;
					if (!world->entityPlayerIDs.contains(entity->id)) continue;

					uint32_t handle = world->entityPlayerIDs.at(entity->id)->handle;

					JSONData::sendPacketClient(world, packet, data, handle);
				}
			}
}

// server-side
void onHealthChange(WorldServer* server, Entity* entity)
{
	nlohmann::json healthData;

	healthData["entityID"] = stl::uuid::to_string(entity->id);
	healthData["health"] = getEntityHealth(entity);

	sendPacketInRegion(server, S_ENTITY_HEALTH_SYNC, healthData, entity->getPos());
}

// client-side
void setEntityHealth(Entity* entity, float health)
{
	stl::string name = entity->getName();
	if (entityInfo.contains(name))
		entityInfo[name].setHealth(entity, health);
}

// client-side
void onEntityHealthSync(fdm::WorldClient* world, fdm::Player* player, const nlohmann::json& data)
{
	auto entityId = stl::uuid()(data["entityID"].get<std::string>());
	float newHealth = data["health"].get<float>();
	fdm::Entity* entity = world->getEntity(entityId);
	if (entity)
		setEntityHealth(entity, newHealth);
}

// client-side
$hook(void, StateIntro, init, StateManager& s)
{
	if (!Config::healthSync()) return original(self, s);

	original(self, s);
	JSONData::SCaddPacketCallback(S_ENTITY_HEALTH_SYNC, onEntityHealthSync);
	init4DMEntities();
}

$hook(void, WorldServer, WorldServer, const stl::path& worldsPath, const stl::path& settingsPath, const stl::path& biomeInfoPath)
{
	init4DMEntities();
	original(self, worldsPath, settingsPath, biomeInfoPath);
}

// server-side
$hook(void, EntityPlayer, update, World* world, double dt)
{
	if (!Config::healthSync()) return original(self, world, dt);

	original(self, world, dt);

	if (world->getType() != World::TYPE_SERVER || !self->player) return;

	WorldServer* server = (WorldServer*)(world);

	static float lastUpdateHealth = self->player->health;
	float thisUpdateHealth = self->player->health;

	if (thisUpdateHealth != lastUpdateHealth)
	{
		onHealthChange(server, self);

		lastUpdateHealth = thisUpdateHealth;
	}
}

// server-side
void Entity_update(Entity* self, World* world, double dt)
{
	static std::unordered_map<Entity*, float> lastHealth{};

	stl::string name = self->getName();
	if (!entityInfo.contains(name)) return;
	auto& info = entityInfo.at(name);

	info.update(self, world, dt);

	if (world->getType() != World::TYPE_SERVER) return;

	auto* server = (WorldServer*)(world);
	
	float curHealth = info.getHealth(self);

	if (!lastHealth.contains(self))
		lastHealth[self] = curHealth;

	if (lastHealth[self] != curHealth)
	{
		onHealthChange(server, self);
		lastHealth[self] = curHealth;
	}
}

extern "C" __declspec(dllexport) bool Health_addEntitySupport(const EntityInfo& info)
{
	if (!Config::healthSync()) return false;

	if (entityInfo.contains(info.name))
		return false;

	auto& e = entityInfo[info.name];
	e.name = info.name;
	e.getHealth = info.getHealth;
	e.setHealth = info.setHealth;

	if (fdm::isServer())
	{
		void* target = (void*&)info.update;
		void** original = reinterpret_cast<void**>(reinterpret_cast<uint64_t>(&e) + offsetof(EntityInfo, update));
		Hook(target, (void*)Entity_update, original);
		EnableHook(target);
	}

	return true;
}

void init4DMEntities()
{
	if (!Config::healthSync()) return;

	Health_addEntitySupport(
		{
			"Player",
			[](Entity* e) -> float { return ((EntityPlayer*)e)->player->health; },
			[](Entity* e, float health) -> void
			{
				if (StateGame::instanceObj.player.EntityPlayerID == e->id)
					StateGame::instanceObj.player.health = health;

				((EntityPlayer*)e)->player->health = health;
			},
			reinterpret_cast<void(*)(Entity*, World*, double)>(fdm::getFuncAddr(Func::EntityPlayer::update))
		});
	Health_addEntitySupport(
		{
			"Spider",
			[](Entity* e) -> float { return ((EntitySpider*)e)->health; },
			[](Entity* e, float health) -> void { ((EntitySpider*)e)->health = health; },
			reinterpret_cast<void(*)(Entity*, World*, double)>(fdm::getFuncAddr(Func::EntitySpider::update))
		});
	Health_addEntitySupport(
		{
			"Butterfly",
			[](Entity* e) -> float { return ((EntityButterfly*)e)->health; },
			[](Entity* e, float health) -> void { ((EntityButterfly*)e)->health = health; },
			reinterpret_cast<void(*)(Entity*, World*, double)>(fdm::getFuncAddr(Func::EntityButterfly::update))
		});
}
