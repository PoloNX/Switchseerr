#include "auth/AuthService.hpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fmt/format.h>
#include <borealis.hpp>

AuthService::AuthService(HttpClient& client, const std::string& serverUrl) : client(client), serverUrl(serverUrl) {}

bool AuthService::login(const std::string& username, const std::string& password) {
    brls::Logger::debug("AuthService: Attempting login for user: {}", username);
    std::string loginUrl = fmt::format("{}/api/v1/auth/jellyfin", serverUrl); // Use the provided server URL

    nlohmann::json body = {
        {"username", username},
        {"password", password}
    };

    try {
        // Authenticate user
        const std::string response = client.post(loginUrl, body.dump());
        const auto loginData = nlohmann::json::parse(response);
        
        if (!loginData.contains("id")) {
            brls::Logger::error("AuthService: Login response missing user ID");
            return false;
        }

        // Login successful, extract user info
        const int userId = loginData["id"].get<int>();
        brls::Logger::debug("AuthService: Login successful, user ID: {}", userId);

        // Retrieve API key from server settings
        const std::string settingsUrl = fmt::format("{}/api/v1/settings/main", serverUrl);
        const std::string settingsResponse = client.get(settingsUrl);
        const auto settings = nlohmann::json::parse(settingsResponse);
        
        if (!settings.contains("apiKey")) {
            brls::Logger::error("AuthService: API key not found in server settings");
            return false;
        }

        // Extract and store API key
        const std::string apiKey = settings["apiKey"].get<std::string>();
        brls::Logger::debug("AuthService: Retrieved API key: {}", apiKey);
        this->setToken(apiKey);

        // Create and save user profile
        const AppUser user = {
            .id = std::to_string(userId),
            .name = loginData["displayName"].get<std::string>(),
            .api_key = apiKey,
            .server_url = serverUrl
        };
        currentUser = user;
        Config::instance().addUser(user, serverUrl);

        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("AuthService: JSON parsing failed: {}", e.what());
    } catch (const nlohmann::json::type_error& e) {
        brls::Logger::error("AuthService: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        brls::Logger::error("AuthService: Login failed: {}", e.what());
    }

    return false;
}

void AuthService::setToken(const std::string& token) {
    authToken = token;
}

std::optional<std::string> AuthService::getToken() const {
    return authToken;
}

void AuthService::attachAuthHeader(struct curl_slist*& headers) const {
    if (authToken.has_value()) {
        std::string header = "Authorization: Bearer " + authToken.value();
        headers = curl_slist_append(headers, header.c_str());
    }
}

void AuthService::logout() {
    authToken.reset();
}