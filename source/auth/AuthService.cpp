#include "auth/AuthService.hpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fmt/format.h>
#include <borealis.hpp>

AuthService::AuthService(std::shared_ptr<HttpClient> client, const std::string& serverUrl) : client(client), serverUrl(serverUrl) {}

bool AuthService::login(const std::string& username, const std::string& password) {
    brls::Logger::debug("AuthService: Attempting login for user: {}", username);
    std::string loginUrl = fmt::format("{}/api/v1/auth/jellyfin", serverUrl); // Use the provided server URL

    nlohmann::json body = {
        {"username", username},
        {"password", password}
    };

    try {
        // Authenticate user with proper headers
        const std::string response = client->post(loginUrl, body.dump());
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
        const std::string settingsResponse = client->get(settingsUrl, nullptr, true);
        const auto settings = nlohmann::json::parse(settingsResponse);
        
        AppUser user = {
            .id = userId,
            .name = loginData["displayName"].get<std::string>(),
            .api_key = "", // API key will be set later if available
            .server_url = serverUrl
        };

        if (settings.contains("apiKey")) {
            // Extract and store API key
            const std::string apiKey = settings["apiKey"].get<std::string>();
            brls::Logger::debug("AuthService: Retrieved API key: {}", apiKey);
            this->setToken(apiKey);
            user.api_key = apiKey; // Set API key in user profile
        }

        // Create and save user profile
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

bool AuthService::loginWithApiKey(const std::string& apiKey) {
    brls::Logger::debug("AuthService: Attempting login with API key");

    if (apiKey.empty()) {
        brls::Logger::error("AuthService: API key is empty");
        return false;
    }

    // Prepare headers for API key authentication
    struct curl_slist* headers = nullptr;
    std::string apiKeyHeader = "X-Api-Key: " + apiKey;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");

    try {
        // Authenticate using API key
        const std::string response = client->get(fmt::format("{}/api/v1/auth/me", serverUrl), headers);
        const auto loginData = nlohmann::json::parse(response);
        
        if (!loginData.contains("id")) {
            brls::Logger::error("AuthService: Login response missing user ID");
            return false;
        }

        // Login successful, extract user info
        const int userId = loginData["id"].get<int>();
        brls::Logger::debug("AuthService: Login successful with API key, user ID: {}", userId);

        // Create and save user profile
        const AppUser user = {
            .id = userId,
            .name = loginData["displayName"].get<std::string>(),
            .api_key = apiKey,
            .server_url = serverUrl
        };
        currentUser = user;
        Config::instance().addUser(user, serverUrl);

        this->setToken(apiKey);
        return true;

    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("AuthService: JSON parsing failed: {}", e.what());
    } catch (const nlohmann::json::type_error& e) {
        brls::Logger::error("AuthService: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        brls::Logger::error("AuthService: Login with API key failed: {}", e.what());
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