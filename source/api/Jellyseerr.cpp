#include "api/Jellyseerr.hpp"
#include "utils/utils.hpp"

#include <borealis.hpp>

namespace jellyseerr {
    std::vector<MediaItem> getLatestMedias(HttpClient& httpClient, const std::string& url, const std::string& apiKey, size_t pageSize) {
        brls::Logger::debug("Jellyseerr, Fetching latest medias from {} with API key", url);

        struct curl_slist* headers = nullptr;
        std::string apiKeyHeader = "X-Api-Key: " + apiKey;
        headers = curl_slist_append(headers, apiKeyHeader.c_str());
        headers = curl_slist_append(headers, "accept: application/json");

        try {
            const std::string response = httpClient.get(fmt::format("{}/api/v1/discover/trending?language=fr", url, pageSize), headers);
            //brls::Logger::debug("Jellyseerr : Response: {}", nlohmann::json::parse(response).dump(4));
            const auto mediasData = nlohmann::json::parse(response);

            if (!mediasData.contains("page")) {
                brls::Logger::error("Jellyseerr: Response missing pageInfo : {}", mediasData.dump(4));
                return {};
            }

            std::vector<MediaItem> medias;

            if (mediasData.contains("results") && mediasData["results"].is_array()) {
                for (const auto& item : mediasData["results"]) {
                    if (item.contains("mediaType")) {
                        const std::string mediaType = item["mediaType"];
                        
                        MediaItem mediaItem;

                        
                        if (mediaType == "movie") {
                            mediaItem.type = MediaType::Movie;
                        } else if (mediaType == "tv") {
                            mediaItem.type = MediaType::Tv;
                        } else {
                            continue; // Skip unknown media types
                        }


                        // Les autres champs nécessiteront une requête TMDb séparée
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

                        mediaItem.voteAverage = get_or_default<double>(item, "voteAverage", 0.0);

                        if (mediaItem.id != -1) {
                            medias.emplace_back(std::move(mediaItem));
                        }
                    }
                }
            }

            curl_slist_free_all(headers);
            return medias;
            
        } catch (const std::exception& e) {
            brls::Logger::error("Jellyseerr : error fetching latest medias: {}", e.what());
            return {};
        }
    }

};