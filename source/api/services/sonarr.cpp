#include "api/services/sonarr.hpp"

#include <fmt/format.h>
#include <borealis/core/logger.hpp>

bool SonarrService::performRequest(const TvRequest &request)
{
    std::string url = fmt::format("{}/api/v1/request", serverUrl);

    nlohmann::json jsonRequest = {
        {"mediaType", "tv"},
        {"mediaId", request.mediaId},
        {"is4k", this->is4k_},
        {"rootFolder", request.rootFolder},
        {"serverId", id},
        {"profileId", request.profileId},
        {"userId", request.userId},
        {"seasons", request.selectedSeasons}};

    std::string json = jsonRequest.dump();
    brls::Logger::debug("SonarrService: Performing request with data: {}", json);

    try
    {
        std::string response = httpClient->post(url, json);;

        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        brls::Logger::debug("RadarrService: Response from request: {}", jsonResponse.dump(4));

        if (jsonResponse.contains("id"))
        {
            return true; // Request was successful
        }
        else
        {
            brls::Logger::error("RadarrService: Request failed, no ID in response : {}", jsonResponse.dump(4));
            return false;
        }
    }
    catch (const std::exception &e)
    {
        brls::Logger::error("RadarrService: Exception during request: {}", e.what());
        return false; // Handle error appropriately
    }
}