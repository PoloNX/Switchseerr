#pragma once

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/MediaItem.hpp"

#include <borealis.hpp>

class VideoCarousel : public brls::Box {
public:
    VideoCarousel(HttpClient& httpClient, AuthService& authService, DiscoverType type);
    ~VideoCarousel();
    
private:
    void doRequest();

    DiscoverType type;
    HttpClient& httpClient;
    AuthService& authService;
    std::string title;
    std::vector<MediaItem> items;

    BRLS_BIND(brls::Header, header, "carousel/header");
    BRLS_BIND(brls::HScrollingFrame, scrollingFrame, "carousel/scroller");
    BRLS_BIND(brls::Box, carouselBox, "carousel/box");
};