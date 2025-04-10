#pragma once

#include <4dm.h>

namespace JSONData
{
	// client->(server)->client (client-side)
	using CSCPacketCallback = std::add_pointer<void(fdm::WorldClient* world, fdm::Player* player, const nlohmann::json& data, const fdm::stl::uuid& from, const fdm::stl::string& fromName)>::type;
	// client->server (server-side)
	using CSPacketCallback = std::add_pointer<void(fdm::WorldServer* world, double dt, const nlohmann::json& data, uint32_t client)>::type;
	// server->client (client-side)
	using SCPacketCallback = std::add_pointer<void(fdm::WorldClient* world, fdm::Player* player, const nlohmann::json& data)>::type;

	inline constexpr fdm::Packet::ClientPacket C_JSON = (fdm::Packet::ClientPacket)0xFFFF;
	inline constexpr fdm::Packet::ServerPacket S_JSON = (fdm::Packet::ServerPacket)0xFFFF;
	inline const fdm::stl::string id = "tr1ngledev.json-data";

	inline bool isLoaded()
	{
		return fdm::isModLoaded(id);
	}

	// client-side
	inline void CSCaddPacketCallback(const fdm::stl::string& packet, CSCPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, CSCPacketCallback)>
			(fdm::getModFuncPointer(id, "addPacketCallback"))
			(packet, callback);
	}
	// client-side
	inline void CSCremovePacketCallback(const fdm::stl::string& packet, CSCPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, CSCPacketCallback)>
			(fdm::getModFuncPointer(id, "removePacketCallback"))
			(packet, callback);
	}
	// client-side
	inline void sendPacketAll(fdm::WorldClient* world, const fdm::stl::string& packet, const nlohmann::json& data, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldClient * world, const fdm::stl::string & packet, const nlohmann::json & data, bool)>
			(fdm::getModFuncPointer(id, "sendPacketAll"))
			(world, packet, data, reliable);
	}
	// client-side
	inline void sendPacketSpecific(fdm::WorldClient* world, const fdm::stl::string& packet, const nlohmann::json& data, const fdm::stl::uuid& target, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldClient * world, const fdm::stl::string & packet, const nlohmann::json & data, const fdm::stl::uuid & target, bool)>
			(fdm::getModFuncPointer(id, "sendPacketSpecific"))
			(world, packet, data, target, reliable);
	}
	// client-side
	inline void sendPacketAllExcept(fdm::WorldClient* world, const fdm::stl::string& packet, const nlohmann::json& data, const fdm::stl::uuid& target, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldClient * world, const fdm::stl::string & packet, const nlohmann::json & data, const fdm::stl::uuid & target, bool)>
			(fdm::getModFuncPointer(id, "sendPacketAllExcept"))
			(world, packet, data, target, reliable);
	}

	// server-side
	inline void CSaddPacketCallback(const fdm::stl::string& packet, CSPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, CSPacketCallback)>
			(fdm::getModFuncPointer(id, "CSaddPacketCallback"))
			(packet, callback);
	}
	// server-side
	inline void CSremovePacketCallback(const fdm::stl::string& packet, CSPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, CSPacketCallback)>
			(fdm::getModFuncPointer(id, "CSremovePacketCallback"))
			(packet, callback);
	}
	// client-side
	inline void sendPacketServer(fdm::WorldClient* world, const fdm::stl::string& packet, const nlohmann::json& data, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldClient * world, const fdm::stl::string & packet, const nlohmann::json & data, bool)>
			(fdm::getModFuncPointer(id, "sendPacketServer"))
			(world, packet, data, reliable);
	}

	// client-side
	inline void SCaddPacketCallback(const fdm::stl::string& packet, SCPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, SCPacketCallback)>
			(fdm::getModFuncPointer(id, "SCaddPacketCallback"))
			(packet, callback);
	}
	// client-side
	inline void SCremovePacketCallback(const fdm::stl::string& packet, SCPacketCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, SCPacketCallback)>
			(fdm::getModFuncPointer(id, "SCremovePacketCallback"))
			(packet, callback);
	}
	// server-side
	inline void sendPacketClient(fdm::WorldServer* world, const fdm::stl::string& packet, const nlohmann::json& data, uint32_t client, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldServer * world, const fdm::stl::string & packet, const nlohmann::json & data, uint32_t, bool)>
			(fdm::getModFuncPointer(id, "sendPacketClient"))
			(world, packet, data, client, reliable);
	}
	// server-side
	inline void broadcastPacket(fdm::WorldServer* world, const fdm::stl::string& packet, const nlohmann::json& data, bool reliable = true)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(fdm::WorldServer * world, const fdm::stl::string & packet, const nlohmann::json & data, bool)>
			(fdm::getModFuncPointer(id, "broadcastPacket"))
			(world, packet, data, reliable);
	}
}
