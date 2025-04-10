#include <4dm.h>
#include "Config.h"

using namespace fdm;

$hook(bool, ItemBlock, action, World* world, Player* player, int action)
{
	if (!Config::noPlantsAtY0()) return original(self, world, player, action);

	if (action == GLFW_MOUSE_BUTTON_RIGHT && player->targetPlaceBlock.y == 0 && player->canPlace && BlockInfo::Blocks[self->blockID].plant)
		return false;

	return original(self, world, player, action);
}
