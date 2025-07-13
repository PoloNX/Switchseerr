#pragma once

#include <vector>
#include <string>
#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/MediaSearchResult.hpp"

class SearchService {
public:
    SearchService(HttpClient& httpClient, AuthService& authService);

    std::vector<MediaSearchResult> search(const std::string& query);

private:
    HttpClient& client;
    AuthService& auth;
};
