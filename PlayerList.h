#pragma once

#pragma once

#include <4dm.h>
#include "Sync.h"

inline constexpr const char* S_PLAYER_LIST = "tr1ngledev.sync.server.player_list";
inline constexpr const char* C_PLAYER_LIST_REQUEST = "tr1ngledev.sync.client.player_list_request";
extern std::vector<std::pair<fdm::stl::uuid, fdm::stl::string>> playerList;
