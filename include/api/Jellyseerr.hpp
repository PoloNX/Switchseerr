#pragma once

#include <nlohmann/json.hpp>
#include <variant>
#include <memory>

#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"

namespace jellyseerr {

    struct PublicSystemInfo {
        std::string version;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PublicSystemInfo, version);

    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, const std::string& apiKey, DiscoverType type,size_t pageSize = 10);
};