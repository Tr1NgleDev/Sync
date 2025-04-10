#pragma once

#include <4dm.h>
#include "Sync.h"

using Sync::Health::EntityInfo;

inline constexpr const char* S_ENTITY_HEALTH_SYNC = "tr1ngledev.sync.server.entity_health_sync";
extern std::unordered_map<fdm::stl::string, EntityInfo> entityInfo;
