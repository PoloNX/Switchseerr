#pragma once

#include <string>
#include <optional>

#include "utils/Config.hpp"
#include "http/HttpClient.hpp"

class AuthService {
public:
    AuthService(HttpClient& client, const std::string& serverUrl);

    bool login(const std::string& username, const std::string& password);

    void setToken(const std::string& token);

    std::string getServerUrl() const {
        return serverUrl;
    }

    std::optional<std::string> getToken() const;

    void attachAuthHeader(struct curl_slist*& headers) const;

    void logout();

private:
    HttpClient& client;
    std::string serverUrl;
    std::optional<std::string> authToken;
    AppUser currentUser;
    bool isTokenValid() const;
};