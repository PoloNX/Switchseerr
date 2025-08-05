#pragma once

#include <nlohmann/json.hpp>
#include <variant>
#include <memory>

#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"
#include "api/services/radarr.hpp"

namespace jellyseerr {

    struct PublicSystemInfo {
        std::string version;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PublicSystemInfo, version);

    void fillDetails(std::shared_ptr<HttpClient> httpClient, MediaItem& mediaItem, const std::string& url);
    std::vector<MediaItem> searchMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, const std::string& query, size_t page = 1);
    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, MediaType type, size_t page = 1);
    std::vector<RadarrService> getRadarrServices(std::shared_ptr<HttpClient> httpClient, const std::string& url);
    std::vector<QualityProfile> getRadarrQualityProfiles(std::shared_ptr<HttpClient> httpClient, const std::string& url, int radarrServiceId);
    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, DiscoverType type,size_t pageSize = 10);
};