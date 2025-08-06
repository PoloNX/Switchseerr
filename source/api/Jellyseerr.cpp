#include "api/Jellyseerr.hpp"
#include "utils/utils.hpp"
#include "utils/ThreadPool.hpp"

#include <borealis.hpp>
#include <future>
#include <thread>

namespace jellyseerr {
    // Fonction helper pour faire une requête de détails pour un seul média
    std::optional<MediaItem> fetchMediaDetails(const std::string& url, 
                                             int tmdbId, MediaType type, int status, std::shared_ptr<HttpClient> httpClient) {
        
        std::string detailsUrl;
        if (type == MediaType::Movie) {
            detailsUrl = fmt::format("{}/api/v1/movie/{}", url, tmdbId);
        } else if (type == MediaType::Tv) {
            detailsUrl = fmt::format("{}/api/v1/tv/{}", url, tmdbId);
        }

        try {
            std::string detailsResponse = httpClient->get(detailsUrl);

            if (detailsResponse.empty()) {
                return std::nullopt;
            }

            const auto detailsData = nlohmann::json::parse(detailsResponse, nullptr, false);
            
            MediaItem mediaItem;
            mediaItem.id = tmdbId;
            mediaItem.type = type;
            mediaItem.overview = get_or_default<std::string>(detailsData, "overview", "");
            mediaItem.posterPath = get_or_default<std::string>(detailsData, "posterPath", "");
            mediaItem.backdropPath = get_or_default<std::string>(detailsData, "backdropPath", "");
            
            if (type == MediaType::Movie) {
                mediaItem.title = get_or_default<std::string>(detailsData, "title", "");
                mediaItem.releaseDate = format_date(get_or_default<std::string>(detailsData, "releaseDate", ""));
            } else if (type == MediaType::Tv) {
                mediaItem.title = get_or_default<std::string>(detailsData, "name", "");
                mediaItem.firstAirDate = format_date(get_or_default<std::string>(detailsData, "firstAirDate", ""));
            }
            
            if (status >= static_cast<int>(MediaStatus::Unknown) && status <= static_cast<int>(MediaStatus::Available)) {
                mediaItem.status = static_cast<MediaStatus>(status);
            } else {
                mediaItem.status = MediaStatus::Unknown;
            }

            mediaItem.voteAverage = get_or_default<double>(detailsData, "voteAverage", 0.0);
            
            return mediaItem;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Failed to fetch details for media ID {}: {}", tmdbId, e.what());
            return std::nullopt;
        }
    }

    std::vector<MediaItem> parseRecentlyAddedResponse(std::shared_ptr<HttpClient> httpClient, const std::string& url, const nlohmann::json& response) {
        std::vector<MediaItem> medias;
        if (!response.contains("results") || !response["results"].is_array()) {
            return medias;
        }

        std::string cookieFilePath = httpClient->getCookieFilePath();

        // Utiliser des shared_ptr pour les promises pour éviter les problèmes de capture
        std::vector<std::shared_ptr<std::promise<std::optional<MediaItem>>>> promises;
        std::vector<std::future<std::optional<MediaItem>>> futures;
        
        auto& threadPool = ThreadPool::instance();
        
        for (const auto& item : response["results"]) {
            if (!item.contains("mediaType")) {
                continue;
            }

            const std::string mediaType = item["mediaType"];
            MediaType type;
            
            if (mediaType == "movie") {
                type = MediaType::Movie;
            } else if (mediaType == "tv") {
                type = MediaType::Tv;
            } else {
                continue;
            }

            int tmdbId = get_or_default<int>(item, "tmdbId", -1);
            if (tmdbId == -1) {
                continue;
            }

            int status = get_or_default<int>(item, "status", static_cast<int>(MediaStatus::Unknown));

            // Créer une promise/future pour cette tâche
            auto promise = std::make_shared<std::promise<std::optional<MediaItem>>>();
            promises.push_back(promise);
            futures.push_back(promise->get_future());
            
            // Soumettre la tâche à la ThreadPool
            threadPool.submit([url, tmdbId, type, status, promise, cookieFilePath](std::shared_ptr<HttpClient> client) {
                try {
                    if (!cookieFilePath.empty()) {
                        client->setCookieFile(cookieFilePath);
                    }
                    auto result = fetchMediaDetails(url, tmdbId, type, status, client);
                    promise->set_value(result);
                } catch (const std::exception& e) {
                    brls::Logger::error("Jellyseerr: Error in ThreadPool task: {}", e.what());
                    promise->set_value(std::nullopt);
                }
            });
        }

        // Récupérer les résultats
        for (auto& future : futures) {
            try {
                auto result = future.get();
                if (result.has_value()) {
                    medias.emplace_back(std::move(result.value()));
                }
            } catch (const std::exception& e) {
                brls::Logger::error("Jellyseerr: Error getting result from future: {}", e.what());
            }
        }

        return medias;
    }

    std::vector<MediaItem> parseDiscoverResponse(const nlohmann::json& response) {
        std::vector<MediaItem> medias;
        if (response.contains("results") && response["results"].is_array()) {
            for (const auto& item : response["results"]) {
                if (item.contains("mediaType")) {
                    const std::string mediaType = item["mediaType"];
                    
                    MediaItem mediaItem;

                    if (mediaType == "movie") {
                        mediaItem.type = MediaType::Movie;
                    } else if (mediaType == "tv") {
                        mediaItem.type = MediaType::Tv;
                    } else {
                        continue;
                    }

                    mediaItem.id = get_or_default<int>(item, "id", -1);

                    

                    if (mediaItem.type == MediaType::Movie) {
                        mediaItem.title = get_or_default<std::string>(item, "title", "");
                    } else if (mediaItem.type == MediaType::Tv) {
                        mediaItem.title = get_or_default<std::string>(item, "name", "");
                    }
                    mediaItem.overview = get_or_default<std::string>(item, "overview", "");
                    mediaItem.posterPath = get_or_default<std::string>(item, "posterPath", "");
                    mediaItem.backdropPath = get_or_default<std::string>(item, "backdropPath", "");

                    if(mediaItem.type == MediaType::Movie) {
                        mediaItem.releaseDate = format_date(get_or_default<std::string>(item, "releaseDate", ""));
                    } else if(mediaItem.type == MediaType::Tv) {
                        mediaItem.firstAirDate = format_date(get_or_default<std::string>(item, "firstAirDate", ""));
                    }

                    auto mediaInfo = get_or_default<nlohmann::json>(item, "mediaInfo", nlohmann::json::object());
                    if (mediaInfo.contains("status")) {
                        int status = mediaInfo["status"].get<int>();
                        // blacklisted status is 6, we skip it
                        if (status == 6) {
                            continue;
                        }
                        else if (status >= static_cast<int>(MediaStatus::Unknown) && status <= static_cast<int>(MediaStatus::Available)) {
                            mediaItem.status = static_cast<MediaStatus>(status);
                        } else {
                            mediaItem.status = MediaStatus::Unknown;
                        }
                    } else {
                        mediaItem.status = MediaStatus::Unknown;
                    }

                    mediaItem.voteAverage = get_or_default<double>(item, "voteAverage", 0.0);

                    if (mediaItem.id != -1) {
                        medias.emplace_back(std::move(mediaItem));
                    }
                }
            }
        }
        return medias;
    }

    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, DiscoverType type, size_t pageSize) {
        brls::Logger::debug("Jellyseerr, Fetching latest medias from {}", url);

        try {
            std::string requestUrl;
            switch(type) {
                case DiscoverType::RecentlyAdded: {
                    requestUrl = fmt::format("{}/api/v1/media?filter=allavailable&take=20&sort=mediaAdded", url);
                    break;
                }
                case DiscoverType::Trending: {
                    requestUrl = fmt::format("{}/api/v1/discover/trending?language=fr", url);
                    break;
                }
                case DiscoverType::PopularMovies: {
                    requestUrl = fmt::format("{}/api/v1/discover/movies?page=1", url);
                    break;
                }
                case DiscoverType::PopularTvShows: {
                    requestUrl = fmt::format("{}/api/v1/discover/tv?page=1", url);
                    break;
                }
                case DiscoverType::FutureMovies: {
                    std::string date = get_date_string();
                    requestUrl = fmt::format("{}/api/v1/discover/movies?page=1&primaryReleaseDateGte={}", url, date);
                    break;
                }
                case DiscoverType::FutureTvShows: {
                    std::string date = get_date_string();
                    requestUrl = fmt::format("{}/api/v1/discover/tv?page=1&firstAirDateGte={}", url, date);
                    break;
                }
            }        

            const std::string response = httpClient->get(requestUrl);
            //brls::Logger::debug("Jellyseerr : Response: {}", nlohmann::json::parse(response).dump(4));
            const auto mediasData = nlohmann::json::parse(response);

            std::vector<MediaItem> medias;
            if (type == DiscoverType::RecentlyAdded) {
                medias = parseRecentlyAddedResponse(httpClient, url, mediasData);
            } else {
                medias = parseDiscoverResponse(mediasData);
            }

            return medias;
            
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr : error fetching latest medias: {}", e.what());
            return {};
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// Radarr specific functions /////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    std::vector<std::shared_ptr<RadarrService>> getRadarrServices(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, const std::string& url) {
        brls::Logger::debug("Jellyseerr: Fetching Radarr services from {}", url);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/service/radarr", url));

            auto servicesData = nlohmann::json::parse(response);
            
            brls::Logger::debug("Jellyseerr: Radarr services response: {}", servicesData.dump(4));

            std::vector<std::shared_ptr<RadarrService>> services;

            for (const auto& item : servicesData) {
                std::shared_ptr<RadarrService> service = std::make_shared<RadarrService>(httpClient, authService);

                // service.qualityProfiles = getRadarrQualityProfiles(httpClient, url, service.id);

                service->setId(item["id"].get<int>());
                service->setServiceName(item["name"].get<std::string>());
                service->setIs4k(item["is4k"].get<bool>());
                service->setIsDefault(item["isDefault"].get<bool>());
                service->setActiveDirectory(item["activeDirectory"].get<std::string>());
                service->setActiveProfileId(item["activeProfileId"].get<int>());
                service->setQualityProfiles(getRadarrQualityProfiles(httpClient, url, service->getId()));

                services.emplace_back(std::move(service));
            }

            return services;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching Radarr services: {}", e.what());
            return {};
        }
    }

    std::vector<QualityProfile> getRadarrQualityProfiles(std::shared_ptr<HttpClient> httpClient, const std::string& url, int radarrServiceId) {
        brls::Logger::debug("Jellyseerr: Fetching Radarr quality profiles for service ID {}", radarrServiceId);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/service/radarr/{}", url, radarrServiceId));

            auto profilesData = nlohmann::json::parse(response);
            std::vector<QualityProfile> profiles;

            brls::Logger::debug("Jellyseerr: Radarr quality profiles response: {}", profilesData.dump(4));

            for (const auto& item : profilesData["profiles"]) {
                brls::Logger::debug("Jellyseerr: Processing Radarr quality profile: {}", item.dump(4));
                QualityProfile profile;
                profile.id = item["id"].get<int>();
                profile.name = item["name"].get<std::string>();

                if(item.contains("rootFolders") && item["rootFolders"].is_array()) {
                    for (const auto& folder : item["rootFolders"]) {
                        RootFolder rootFolder;
                        rootFolder.id = folder["id"].get<int>();
                        rootFolder.freeSpace = folder["freeSpace"].get<int>();
                        rootFolder.path = folder["path"].get<std::string>();
                        profile.rootFolders.push_back(std::move(rootFolder));
                    }
                }

                profiles.emplace_back(std::move(profile));
            }

            return profiles;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching Radarr quality profiles: {}", e.what());
            return {};
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// Sonarr specific functions /////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    std::vector<std::shared_ptr<SonarrService>> getSonarrServices(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, const std::string& url) {
        brls::Logger::debug("Jellyseerr: Fetching Sonarr services from {}", url);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/service/sonarr", url));

            auto servicesData = nlohmann::json::parse(response);

            brls::Logger::debug("Jellyseerr: Sonarr services response: {}", servicesData.dump(4));

            std::vector<std::shared_ptr<SonarrService>> services;

            for (const auto& item : servicesData) {
                std::shared_ptr<SonarrService> service = std::make_shared<SonarrService>(httpClient, authService);

                // service.qualityProfiles = getSonarrQualityProfiles(httpClient, url, service.id);

                service->setId(item["id"].get<int>());
                service->setServiceName(item["name"].get<std::string>());
                service->setIs4k(item["is4k"].get<bool>());
                service->setIsDefault(item["isDefault"].get<bool>());
                service->setActiveDirectory(item["activeDirectory"].get<std::string>());
                service->setActiveProfileId(item["activeProfileId"].get<int>());
                service->setQualityProfiles(getRadarrQualityProfiles(httpClient, url, service->getId()));

                services.emplace_back(std::move(service));
            }

            return services;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching Radarr services: {}", e.what());
            return {};
        }
    }

    std::vector<QualityProfile> getSonarrQualityProfiles(std::shared_ptr<HttpClient> httpClient, const std::string& url, int sonarrServiceId) {
        brls::Logger::debug("Jellyseerr: Fetching Sonarr quality profiles for service ID {}", sonarrServiceId);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/service/sonarr/{}", url, sonarrServiceId));

            auto profilesData = nlohmann::json::parse(response);
            std::vector<QualityProfile> profiles;

            brls::Logger::debug("Jellyseerr: Sonarr quality profiles response: {}", profilesData.dump(4));

            for (const auto& item : profilesData["profiles"]) {
                brls::Logger::debug("Jellyseerr: Processing Sonarr quality profile: {}", item.dump(4));
                QualityProfile profile;
                profile.id = item["id"].get<int>();
                profile.name = item["name"].get<std::string>();

                if(item.contains("rootFolders") && item["rootFolders"].is_array()) {
                    for (const auto& folder : item["rootFolders"]) {
                        RootFolder rootFolder;
                        rootFolder.id = folder["id"].get<int>();
                        rootFolder.freeSpace = folder["freeSpace"].get<int>();
                        rootFolder.path = folder["path"].get<std::string>();
                        profile.rootFolders.push_back(std::move(rootFolder));
                    }
                }

                profiles.emplace_back(std::move(profile));
            }

            return profiles;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching Radarr quality profiles: {}", e.what());
            return {};
        }
    }

    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, MediaType type, size_t page) {
        brls::Logger::debug("Jellyseerr: Fetching medias from {} with API key", url);

        try {
            std::string requestUrl;
            if (type == MediaType::Movie) {
                requestUrl = fmt::format("{}/api/v1/discover/movies?page={}", url, page);
            } else if (type == MediaType::Tv) {
                requestUrl = fmt::format("{}/api/v1/discover/tv?page={}", url, page);
            } else {
                throw std::invalid_argument("Unsupported media type");
            }

            const std::string response = httpClient->get(requestUrl);
            //brls::Logger::debug("Jellyseerr : Response: {}", nlohmann::json::parse(response).dump(4));
            const auto mediasData = nlohmann::json::parse(response);

            return parseDiscoverResponse(mediasData);
            
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr : error fetching medias: {}", e.what());
            return {};
        }
    }

    std::vector<MediaItem> searchMedias(std::shared_ptr<HttpClient> httpClient, const std::string &url, const std::string &query, size_t page) {
        brls::Logger::debug("Jellyseerr: Searching medias with query '{}' on {}", query, url);

        try {
            std::string requestUrl = fmt::format("{}/api/v1/search?query={}&page={}", url, query, page);
            const std::string response = httpClient->get(requestUrl);

            if (response.empty()) {
                brls::Logger::warning("Jellyseerr: No results found for query '{}'", query);
                return {};
            }

            const auto searchData = nlohmann::json::parse(response);
            //parsing the search results
            auto medias = parseDiscoverResponse(searchData);

            return medias;

        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error searching medias: {}", e.what());
            return {};
        }
    }

    void fillDetails(std::shared_ptr<HttpClient> httpClient, MediaItem& mediaItem, const std::string& url) {
        brls::Logger::debug("Jellyseerr: Filling details for media item ID: {}", mediaItem.id);
        brls::Logger::debug("Jellyseerr: Media type: {}, Media name: {}", mediaItem.type == MediaType::Movie ? "Movie" : "TV Show", mediaItem.title);
        try {
            std::string detailsUrl;
            if (mediaItem.type == MediaType::Movie) {
                detailsUrl = fmt::format("{}/api/v1/movie/{}", url, mediaItem.id);
            } else {
                detailsUrl = fmt::format("{}/api/v1/tv/{}", url, mediaItem.id);
            }

            std::string response = httpClient->get(detailsUrl);

            if (response.empty()) {
                brls::Logger::warning("Jellyseerr: No details found for media item ID: {}", mediaItem.id);
                return;
            }

            const auto detailsData = nlohmann::json::parse(response, nullptr, false);

            // Log the response to debug
            //brls::Logger::debug("Jellyseerr: Details response: {}", detailsData.dump(4));

            // Update common fields
            mediaItem.overview = get_or_default<std::string>(detailsData, "overview", "");
            mediaItem.posterPath = get_or_default<std::string>(detailsData, "posterPath", "");
            mediaItem.backdropPath = get_or_default<std::string>(detailsData, "backdropPath", "");
            mediaItem.originalLanguage = get_or_default<std::string>(detailsData, "originalLanguage", "");
            
            // Handle genres properly - they might be an array of objects or strings
            if (detailsData.contains("genres") && detailsData["genres"].is_array()) {
                std::vector<std::string> genres;
                for (const auto& genre : detailsData["genres"]) {
                    if (genre.is_string()) {
                        genres.push_back(genre.get<std::string>());
                    } else if (genre.is_object() && genre.contains("name")) {
                        genres.push_back(genre["name"].get<std::string>());
                    }
                }
                mediaItem.genres = genres;
            } else {
                mediaItem.genres = {};
            }

            // Update type-specific fields - use the original type, not the parsed data
            if (mediaItem.type == MediaType::Movie) {
                brls::Logger::debug("Jellyseerr: Processing movie details for item name: {}", mediaItem.title);
                
                // Only update title if it's not empty in the response
                std::string newTitle = get_or_default<std::string>(detailsData, "title", "");
                if (!newTitle.empty()) {
                    mediaItem.title = newTitle;
                }
                
                mediaItem.releaseDate = format_date(get_or_default<std::string>(detailsData, "releaseDate", ""));
                mediaItem.originalTitle = get_or_default<std::string>(detailsData, "originalTitle", "");
                mediaItem.revenue = get_or_default<int>(detailsData, "revenue", 0);
                mediaItem.runtime = get_or_default<int>(detailsData, "runtime", 0);
                mediaItem.statusString = get_or_default<std::string>(detailsData, "statusString", "");
            } else if (mediaItem.type == MediaType::Tv) {
                brls::Logger::debug("Jellyseerr: Processing TV details for item name: {}", mediaItem.title);
                
                // Only update title if it's not empty in the response
                std::string newTitle = get_or_default<std::string>(detailsData, "name", "");
                if (!newTitle.empty()) {
                    mediaItem.title = newTitle;
                }
                
                mediaItem.firstAirDate = format_date(get_or_default<std::string>(detailsData, "firstAirDate", ""));
                mediaItem.inProduction = get_or_default<bool>(detailsData, "inProduction", false);
                mediaItem.lasAirDate = format_date(get_or_default<std::string>(detailsData, "lastAirDate", ""));
                mediaItem.numberOfEpisodes = get_or_default<int>(detailsData, "numberOfEpisodes", 0);
                mediaItem.numberOfSeasons = get_or_default<int>(detailsData, "numberOfSeasons", 0);
                mediaItem.originalName = get_or_default<std::string>(detailsData, "originalName", "");

                // Parse seasons if available
                if (detailsData.contains("seasons") && detailsData["seasons"].is_array()) {
                    for (const auto& season : detailsData["seasons"]) {
                        brls::Logger::debug("Jellyseerr: Processing season: {}", season.dump(4));
                        Season seasonItem;
                        seasonItem.airDate = get_or_default<std::string>(season, "airDate", "");
                        seasonItem.episodeCount = get_or_default<int>(season, "episodeCount", 0);
                        seasonItem.id = get_or_default<int>(season, "seasonNumber", 0);
                        seasonItem.tvId = mediaItem.id; // Assuming tvId is the same as mediaItem.id
                        seasonItem.name = get_or_default<std::string>(season, "name", "");
                        seasonItem.overview = get_or_default<std::string>(season, "overview", "");
                        seasonItem.seasonNumber = get_or_default<int>(season, "seasonNumber", 0);
                        seasonItem.posterPath = get_or_default<std::string>(season, "posterPath", "");

                        mediaItem.seasons.push_back(std::move(seasonItem));
                    }
                }

                // Fill season status (available, pending, ...). This code is a garbage, but it works, idk how jellyseerr handle this by default


                if (detailsData.contains("mediaInfo") && detailsData["mediaInfo"].contains("seasons") && detailsData["mediaInfo"]["seasons"].is_array()) {
                    for (const auto& season : detailsData["mediaInfo"]["seasons"]) {
                        int seasonNumber = get_or_default<int>(season, "seasonNumber", 0);
                        MediaStatus status = static_cast<MediaStatus>(get_or_default<int>(season, "status", static_cast<int>(MediaStatus::Unknown)));
                        
                        auto it = std::find_if(mediaItem.seasons.begin(), mediaItem.seasons.end(),
                            [seasonNumber](const Season& s) { return s.seasonNumber == seasonNumber; });
                        
                        if (it != mediaItem.seasons.end()) {
                            brls::Logger::debug("Jellyseerr: Setting status for season {} to {}", seasonNumber, static_cast<int>(status));
                            it->status = status;
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching media item details for ID {}: {}", mediaItem.id, e.what());
        }
    }

    PublicServerInfo getPublicServerInfo(std::shared_ptr<HttpClient> httpClient, const std::string& url) {
        brls::Logger::debug("Jellyseerr: Fetching public server info from {}", url);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/settings/public", url));
            auto serverInfoData = nlohmann::json::parse(response);

            PublicServerInfo serverInfo;
            serverInfo.applicationTitle = get_or_default<std::string>(serverInfoData, "applicationTitle", "Jellyseerr");
            serverInfo.localLogin = get_or_default<bool>(serverInfoData, "localLogin", false);
            serverInfo.mediaServerLogin = get_or_default<bool>(serverInfoData, "mediaServerLogin", false);

            // 1 : Plex, 2 : Jellyfin
            int serverTypeStr = get_or_default<int>(serverInfoData, "mediaServerType", 0);
            if (serverTypeStr == 1) {
                serverInfo.serverType = ConnectionServer::PLEX;
            } else {
                serverInfo.serverType = ConnectionServer::JELLYFIN;
            }

            return serverInfo;
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching public server info: {}", e.what());
            return {};
        }
    }

    void fillSeasonDetails(std::shared_ptr<HttpClient> httpClient, const std::string& url, Season& season) {
        brls::Logger::debug("Jellyseerr: Fetching details for season ID: {}", season.id);

        try {
            std::string response = httpClient->get(fmt::format("{}/api/v1/tv/{}/season/{}", url, season.tvId, season.id));
            auto seasonData = nlohmann::json::parse(std::move(response), nullptr, false);
            if (!seasonData.contains("episodes")) {
                brls::Logger::warning("Jellyseerr: No episodes found for season ID: {}", season.id);
                return;
            }

            for (const auto& episodeData : seasonData["episodes"]) {
                Episode episode;
                episode.id = get_or_default<int>(episodeData, "id", 0);
                episode.name = get_or_default<std::string>(episodeData, "name", "");
                episode.airDate = format_date(get_or_default<std::string>(episodeData, "airDate", ""));
                episode.episodeNumber = get_or_default<int>(episodeData, "episodeNumber", 0);
                episode.overview = get_or_default<std::string>(episodeData, "overview", "");
                episode.stillPath = get_or_default<std::string>(episodeData, "stillPath", "");
                season.episodes.push_back(std::move(episode));
            }

        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr: Error fetching season details for ID {}: {}", season.id, e.what());
        }
    }

};