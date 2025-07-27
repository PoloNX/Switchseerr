#pragma once

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/MediaItem.hpp"
#include "view/VideoCard.hpp"

#include <borealis.hpp>
#include <memory>

class VideoCarousel : public brls::Box {
public:
    VideoCarousel(std::shared_ptr<HttpClient> httpClient, AuthService& authService, DiscoverType type);
    ~VideoCarousel();
    
    void doRequest();

private:
    DiscoverType type;
    std::shared_ptr<HttpClient> httpClient;
    AuthService& authService;
    std::string title;
    std::string serverUrl;
    std::string apiKey;

    void configureHeaderTitle();
    void createAndAddVideoCard(MediaItem& item);
    void setupVideoCardContent(VideoCardCell* videoCard, const MediaItem& item);
    void setupVideoCardStyling(VideoCardCell* videoCard, const MediaItem& item);
    void setupVideoCardInteractions(VideoCardCell* videoCard, MediaItem& item);
    void loadVideoCardImage(VideoCardCell* videoCard, const MediaItem& item);
    void addVideoCardToCarousel(VideoCardCell* videoCard);

    BRLS_BIND(brls::Header, header, "carousel/header");
    BRLS_BIND(brls::HScrollingFrame, scrollingFrame, "carousel/scroller");
    BRLS_BIND(brls::Box, carouselBox, "carousel/box");
};