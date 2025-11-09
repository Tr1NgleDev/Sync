#include <4dm.h>
#include "Config.h"
#include <fstream>

using namespace fdm;

nlohmann::json Config::config
{
	{ "daytime", true },
	{ "playerList", true },
	{ "healthSync", true },
	{ "movementUpdateFix", true },
	{ "netOptimization", true },
	{ "clientBlockUpdate", true },
	{ "instaMiningFix", true },
	{ "blockReplaceFix", true },
	{ "noPlantsAtY0", true },
};

inline static void getToggle(const nlohmann::json& j, const std::string& key)
{
	if (j.contains(key) && j[key].is_boolean())
		Config::config[key] = j[key];
}

void loadConfig()
{
	stl::string configPath = std::format("{}/config.json", fdm::getModPath(fdm::modID));
	if (!std::filesystem::exists(configPath))
	{
		std::ofstream configOut{ configPath };
		if (configOut.is_open())
		{
			configOut << Config::config.dump(4);
			configOut.close();
		}
	}

	std::ifstream configIn{ configPath };
	if (!configIn.is_open()) return;

	nlohmann::json j = nlohmann::json::parse(configIn);

	if (j.is_object())
	{
		getToggle(j, "daytime");
		getToggle(j, "playerList");
		getToggle(j, "healthSync");
		getToggle(j, "movementUpdateFix");
		getToggle(j, "netOptimization");
		getToggle(j, "clientBlockUpdate");
		getToggle(j, "instaMiningFix");
		getToggle(j, "blockReplaceFix");
		getToggle(j, "noPlantsAtY0");
	}

	configIn.close();
}

$hookStatic(int, main_cpp, main)
{
	loadConfig();
	return original();
}
