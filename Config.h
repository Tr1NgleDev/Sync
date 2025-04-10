#pragma once

#include <4dm.h>

namespace Config
{
	extern nlohmann::json config;
	inline bool daytime()
	{
		return config["daytime"];
	}
	inline bool playerList()
	{
		return config["playerList"];
	}
	inline bool healthSync()
	{
		return config["healthSync"];
	}
	inline bool movementUpdateFix()
	{
		return config["movementUpdateFix"];
	}
	inline bool netOptimization()
	{
		return config["netOptimization"];
	}
	inline bool clientBlockUpdate()
	{
		return config["clientBlockUpdate"];
	}
	inline bool instaMiningFix()
	{
		return config["instaMiningFix"];
	}
	inline bool blockReplaceFix()
	{
		return config["blockReplaceFix"];
	}
	inline bool noPlantsAtY0()
	{
		return config["noPlantsAtY0"];
	}
}
