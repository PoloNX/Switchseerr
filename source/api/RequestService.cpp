#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "api/RequestService.hpp"

RequestService::RequestService(HttpClient& httpClient, AuthService& authService)
    : client(httpClient), auth(authService) {}

std::optional<int> RequestService::createRequest(const MovieRequest& request) { 
    std::string url = fmt::format("{}{}", "http://jellyseerr.cabanaflix.ovh", BASE_REQUEST_URL);

    // Create the JSON to perform the request
    nlohmann::json jsonRequest = {
        {"mediaType", mediaTypeToString(request.type)},
        {"mediaId", request.mediaId},
        {"tvdbId", request.tvdbId},
        {"seasons", request.seasons},
        {"is4k", request.is4k},
        {"serverId", request.serverId},
        {"profilerId", request.profilerId},
        {"rootFolder", request.rootFolder},
        {"languageProfileId", request.languageProfileId},
        {"userId", request.userId}
    };

    // Convert the JSON to a string
    std::string jsonData = jsonRequest.dump();

    try {
        // Perform the POST request
        std::string response = client.post(url, jsonData);

        // Parse the response JSON
        nlohmann::json jsonResponse = nlohmann::json::parse(response);

        // Check if the request was successful and return the request ID
        if (jsonResponse.contains("id")) {
            return jsonResponse["id"].get<int>();
        } else {
            return std::nullopt;  // Request failed
        }
    } catch (const std::exception& e) {
        fmt::print("Error creating request: {}\n", e.what());
        return std::nullopt;  // Handle error
    }
}