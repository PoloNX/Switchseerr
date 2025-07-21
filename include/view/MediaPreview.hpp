#pragma once

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/MediaItem.hpp"

#include <borealis.hpp>

class MediaPreview : public brls::Box {
public:
    MediaPreview(HttpClient& httpClient, AuthService& authService, MediaItem& mediaItem);

private:
    HttpClient& httpClient;
    AuthService& authService;
    MediaItem& mediaItem;

    BRLS_BIND(brls::Image, posterImage, "preview/video/poster");
    BRLS_BIND(brls::Image, backdropImage, "preview/video/backdrop");
    BRLS_BIND(brls::Label, statusLabel, "preview/video/status");
    BRLS_BIND(brls::Label, titleLabel, "preview/video/title");
    BRLS_BIND(brls::Label, durationLabel, "preview/video/duration");
    BRLS_BIND(brls::Label, genreLabel, "preview/video/genre");
    BRLS_BIND(brls::Button, favoriteButton, "preview/video/favorite");
    BRLS_BIND(brls::Button, requestButton, "preview/video/request");
    BRLS_BIND(brls::Button, reportButton, "preview/video/report");
    BRLS_BIND(brls::Label, overviewLabel, "preview/video/overview");
};