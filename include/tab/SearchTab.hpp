#pragma once

#include <borealis.hpp>

#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"
#include "models/MediaItem.hpp"

class SearchTab : public brls::Box {
public:
    SearchTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService);
    ~SearchTab() override;

private:
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;
    MediaType mediaType;

    BRLS_BIND(brls::Box, content, "search/content");
};
