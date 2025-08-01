#pragma once

#include <borealis.hpp>

#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"
#include "models/MediaItem.hpp"

class MediaTab : public brls::Box {
public:
    MediaTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType mediaType);
    ~MediaTab() override;

private:
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;
    MediaType mediaType;

    BRLS_BIND(brls::Box, content, "movie/content");
};
