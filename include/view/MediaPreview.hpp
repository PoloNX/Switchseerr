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
};