#include <4dm.h>
#include "Config.h"

using namespace fdm;

$hook(bool, WorldServer, setBlockUpdate, const glm::ivec4& block, uint8_t value)
{
	if (!Config::blockReplaceFix()) return original(self, block, value);

	uint8_t curValue = self->getBlock(block);
	if (value == curValue) return false;
	if (curValue != BlockInfo::AIR && value != BlockInfo::AIR) return false;
	return original(self, block, value);
}
