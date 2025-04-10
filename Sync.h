#pragma once

#include <4dm.h>

namespace Sync
{
	inline const fdm::stl::string id = "tr1ngledev.sync";

	inline bool isLoaded()
	{
		return fdm::isModLoaded(id);
	}

	namespace Health
	{
		struct EntityInfo
		{
			fdm::stl::string name;
			float(*getHealth)(fdm::Entity* e);
			void(*setHealth)(fdm::Entity* e, float health);
			void(*update)(fdm::Entity* self, fdm::World* world, double dt);
		};
		/*
		returns false if an entity with the name has already been added.
		example usage (EntityJellyfish):

		Sync::Health::addEntitySupport(
		{
			"Jellyfish",																							// what getName() returns
			[](Entity* e) -> float { return ((EntityJellyfish*)e)->health; },										// get health
			[](Entity* e, float health) -> void { ((EntityJellyfish*)e)->health = health; },						// set health
			reinterpret_cast<void(*)(Entity*, World*, double)>(fdm::getClassFuncAddr<&EntityJellyfish::update>())	// for hooking
		});
		*/
		inline bool addEntitySupport(const EntityInfo& entityInfo)
		{
			if (!isLoaded())
				return false;
			return reinterpret_cast<bool(__stdcall*)(const EntityInfo & entityInfo)>
				(fdm::getModFuncPointer(id, "Health_addEntitySupport"))
				(entityInfo);
		}
	}

	namespace PlayerList
	{
		inline const std::vector<std::pair<fdm::stl::uuid, fdm::stl::string>>& getPlayerList()
		{
			if (!isLoaded())
				return {};
			return reinterpret_cast<const std::vector<std::pair<fdm::stl::uuid, fdm::stl::string>>& (__stdcall*)()>
				(fdm::getModFuncPointer(id, "PlayerList_getPlayerList"))
				();
		}
		inline void requestPlayerList(fdm::WorldClient* world)
		{
			if (!isLoaded())
				return;
			return reinterpret_cast<void(__stdcall*)(fdm::WorldClient * world)>
				(fdm::getModFuncPointer(id, "PlayerList_requestPlayerList"))
				(world);
		}
	}
}
