#pragma once

#include <nlohmann/json.hpp>
#include <variant>
#include <memory>

#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"
#include "api/services/sonarr.hpp"
#include "api/services/radarr.hpp"

namespace jellyseerr {

    enum class ConnectionServer {
        JELLYFIN,
        PLEX,
        LOCAL
    };

    struct PublicServerInfo {
        std::string applicationTitle;
        bool localLogin;
        bool mediaServerLogin;
        ConnectionServer serverType;
    };

    struct PublicSystemInfo {
        std::string version;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PublicSystemInfo, version);

    PublicServerInfo getPublicServerInfo(std::shared_ptr<HttpClient> httpClient, const std::string& url);
    void fillDetails(std::shared_ptr<HttpClient> httpClient, MediaItem& mediaItem, const std::string& url);
    std::vector<MediaItem> searchMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, const std::string& query, size_t page = 1);
    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, MediaType type, size_t page = 1);

    //radarr specific functions
    std::vector<std::shared_ptr<RadarrService>> getRadarrServices(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, const std::string& url);
    std::vector<QualityProfile> getRadarrQualityProfiles(std::shared_ptr<HttpClient> httpClient, const std::string& url, int radarrServiceId);
    
    //sonarr specific functions
    std::vector<std::shared_ptr<SonarrService>> getSonarrServices(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, const std::string& url);
    std::vector<QualityProfile> getSonarrQualityProfiles(std::shared_ptr<HttpClient> httpClient, const std::string& url, int sonarrServiceId);

    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, DiscoverType type,size_t pageSize = 10);
    void fillSeasonDetails(std::shared_ptr<HttpClient> httpClient, const std::string& url, Season& season);
};