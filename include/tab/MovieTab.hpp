#pragma once

#include <borealis.hpp>

#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"

class MovieTab : public brls::Box {
public:
    MovieTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService);
    ~MovieTab() override;

private:
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;

    BRLS_BIND(brls::Box, content, "movie/content");
};
