#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <borealis/core/logger.hpp>
#include "api/RequestService.hpp"

RequestService::RequestService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : client(httpClient), auth(authService) {}

bool RequestService::createRequest(const MovieRequest& request, MediaType mediaType) { 
    std::string url = fmt::format("{}{}", auth->getServerUrl(), BASE_REQUEST_URL);


    nlohmann::json jsonRequest;

    // Create the JSON to perform the request
    switch (mediaType) {
        case MediaType::Movie:
            jsonRequest = {
                {"mediaType", mediaTypeToString(request.type)},
                {"mediaId", request.mediaId},
                {"is4k", request.is4k},
                {"serverId", request.serverId},
                {"profileId", request.profilerId},
                {"rootFolder", request.rootFolder},
                {"userId", request.userId}
            };
            break;
        case MediaType::Tv:
            jsonRequest = {
                {"mediaType", mediaTypeToString(request.type)},
                {"tvdbId", request.tvdbId},
                {"seasons", request.seasons},
                {"is4k", request.is4k},
                {"serverId", request.serverId},
                {"profileId", request.profilerId},
                {"rootFolder", request.rootFolder},
                {"languageProfileId", request.languageProfileId},
                {"userId", request.userId}
            };
            break;
        default:
            brls::Logger::error("RequestService: Unsupported media type for request creation");
            return false;
    }

    // Convert the JSON to a string
    std::string jsonData = jsonRequest.dump();
    brls::Logger::debug("RequestService: Creating request with data: {}", jsonData);
    

    try {
        // Perform the POST request
        std::string response = client->post(url, jsonData);
        
        // Parse the response JSON
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        brls::Logger::debug("RequestService: Response from request creation: {}", jsonResponse.dump(4));

        // Check if the request was successful and return the request ID
        if (jsonResponse.contains("id")) {
            return true;
        } else {
            return false;  // Request failed
        }
    } catch (const std::exception& e) {
        fmt::print("Error creating request: {}\n", e.what());
        return true;  // Handle error
    }
}