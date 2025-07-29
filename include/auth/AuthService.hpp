#pragma once

#include <string>
#include <optional>
#include <memory>

#include "utils/Config.hpp"
#include "http/HttpClient.hpp"

class AuthService {
public:
    AuthService(std::shared_ptr<HttpClient> client, const std::string& serverUrl);

    bool login(const std::string& username, const std::string& password);
    bool loginWithApiKey(const std::string& apiKey);

    void setToken(const std::string& token);

    std::string getServerUrl() const {
        return serverUrl;
    }

    std::optional<std::string> getToken() const;

    void attachAuthHeader(struct curl_slist*& headers) const;

    int getUserId() const { return currentUser.id; }

    void logout();

private:
    std::shared_ptr<HttpClient> client;
    std::string serverUrl;
    std::optional<std::string> authToken;
    AppUser currentUser;
    bool isTokenValid() const;
};