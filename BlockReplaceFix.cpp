#include <4dm.h>
#include "Config.h"

using namespace fdm;

$hook(void, WorldServer, setBlockUpdate, const glm::ivec4& block, uint8_t value)
{
	if (!Config::blockReplaceFix()) return original(self, block, value);

	uint8_t curValue = self->getBlock(block);
	if (value == curValue) return;
	if (curValue != BlockInfo::AIR && curValue != BlockInfo::LAVA && value != BlockInfo::AIR) return;

	return original(self, block, value);
}
