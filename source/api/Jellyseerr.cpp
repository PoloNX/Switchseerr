#include "api/Jellyseerr.hpp"
#include "utils/utils.hpp"
#include "utils/ThreadPool.hpp"

#include <borealis.hpp>
#include <future>
#include <thread>

namespace jellyseerr {
    // Fonction helper pour faire une requête de détails pour un seul média
    std::optional<MediaItem> fetchMediaDetails(const std::string& url, const std::string& apiKey, 
                                             int tmdbId, MediaType type, int status, std::shared_ptr<HttpClient> httpClient) {
        
        std::string detailsUrl;
        if (type == MediaType::Movie) {
            detailsUrl = fmt::format("{}/api/v1/movie/{}", url, tmdbId);
        } else if (type == MediaType::Tv) {
            detailsUrl = fmt::format("{}/api/v1/tv/{}", url, tmdbId);
        }

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, fmt::format("X-Api-Key: {}", apiKey).c_str());

        try {
            std::string detailsResponse = httpClient->get(detailsUrl, headers, false);
            curl_slist_free_all(headers);

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
                mediaItem.releaseDate = get_or_default<std::string>(detailsData, "releaseDate", "");
            } else if (type == MediaType::Tv) {
                mediaItem.title = get_or_default<std::string>(detailsData, "name", "");
                mediaItem.firstAirDate = get_or_default<std::string>(detailsData, "firstAirDate", "");
            }
            
            if (status >= static_cast<int>(MediaStatus::Unknown) && status <= static_cast<int>(MediaStatus::Available)) {
                mediaItem.status = static_cast<MediaStatus>(status);
            } else {
                mediaItem.status = MediaStatus::Unknown;
            }

            mediaItem.voteAverage = get_or_default<double>(detailsData, "voteAverage", 0.0);
            
            return mediaItem;
        } catch (const std::exception& e) {
            curl_slist_free_all(headers);
            brls::Logger::error("Jellyseerr: Failed to fetch details for media ID {}: {}", tmdbId, e.what());
            return std::nullopt;
        }
    }

    std::vector<MediaItem> parseRecentlyAddedResponse(std::shared_ptr<HttpClient> httpClient, const std::string& url, const std::string& apiKey, const nlohmann::json& response) {
        std::vector<MediaItem> medias;
        if (!response.contains("results") || !response["results"].is_array()) {
            return medias;
        }

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
            threadPool.submit([url, apiKey, tmdbId, type, status, promise](std::shared_ptr<HttpClient> client) {
                try {
                    auto result = fetchMediaDetails(url, apiKey, tmdbId, type, status, client);
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
                        mediaItem.releaseDate = get_or_default<std::string>(item, "releaseDate", "");
                    } else if(mediaItem.type == MediaType::Tv) {
                        mediaItem.firstAirDate = get_or_default<std::string>(item, "firstAirDate", "");
                    }

                    auto mediaInfo = get_or_default<nlohmann::json>(item, "mediaInfo", nlohmann::json::object());
                    if (mediaInfo.contains("status")) {
                        int status = mediaInfo["status"].get<int>();
                        if (status >= static_cast<int>(MediaStatus::Unknown) && status <= static_cast<int>(MediaStatus::Available)) {
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

    std::vector<MediaItem> getMedias(std::shared_ptr<HttpClient> httpClient, const std::string& url, const std::string& apiKey, DiscoverType type, size_t pageSize) {
        brls::Logger::debug("Jellyseerr, Fetching latest medias from {} with API key", url);

        struct curl_slist* headers = nullptr;
        std::string apiKeyHeader = "X-Api-Key: " + apiKey;
        headers = curl_slist_append(headers, apiKeyHeader.c_str());
        headers = curl_slist_append(headers, "accept: application/json");

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

            const std::string response = httpClient->get(requestUrl, headers);
            curl_slist_free_all(headers);
            //brls::Logger::debug("Jellyseerr : Response: {}", nlohmann::json::parse(response).dump(4));
            const auto mediasData = nlohmann::json::parse(response);

            std::vector<MediaItem> medias;
            if (type == DiscoverType::RecentlyAdded) {
                medias = parseRecentlyAddedResponse(httpClient, url, apiKey, mediasData);
            } else {
                medias = parseDiscoverResponse(mediasData);
            }

            return medias;
            
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr : error fetching latest medias: {}", e.what());
            return {};
        }
    }

};