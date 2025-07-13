#pragma once

#include <nlohmann/json.hpp>

namespace jellyseerr {

    struct PublicSystemInfo {
        std::string version;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PublicSystemInfo, version);
};