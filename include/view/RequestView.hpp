#pragma once

#include "models/MediaItem.hpp"
#include "auth/AuthService.hpp"
#include "api/services/radarr.hpp"
#include "http/HttpClient.hpp"

#include <borealis.hpp>

class RequestView : public brls::Box {
public:
    RequestView(std::shared_ptr<HttpClient> httpClient, MediaItem mediaItem, std::shared_ptr<AuthService> authService);

    void offsetTick();
    brls::Animatable showOffset = 0;
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;
    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;

    void loadImage();
    void loadProfiles();
    void loadButtonActions();

    bool isTranslucent() override
    {
        return true;
    }
private:
    MediaItem mediaItem;
    std::shared_ptr<HttpClient> client;
    std::shared_ptr<AuthService> authService;

    QualityProfile selectedQualityProfile;
    std::shared_ptr<BaseService> selectedService;

    std::vector<std::shared_ptr<BaseService>> availableServers;

    void loadQualityProfiles();
    void loadServerProfiles();
    void loadSeasons();

    BRLS_BIND(brls::Box, shadowImage, "request/image/background");
    BRLS_BIND(brls::Image, backdropImage, "request/image");
    BRLS_BIND(brls::Label, titleLabel, "request/title");
    BRLS_BIND(brls::Label, mediaLabel, "request/media");

    // Details
    BRLS_BIND(brls::Box, detailsBox, "request/details");
    BRLS_BIND(brls::Header, serverHeader, "request/server/header");
    BRLS_BIND(brls::DetailCell, serverCell, "request/server");
    BRLS_BIND(brls::Header, qualityHeader, "request/quality/header");
    BRLS_BIND(brls::DetailCell, qualityCell, "request/quality");

    // For TV Shows only
    BRLS_BIND(brls::Header, seasonHeader, "request/seasons/header");
    BRLS_BIND(brls::Box, seasonContent, "request/seasons/content");
    BRLS_BIND(brls::Box, seasonBox, "request/seasons/box");
    BRLS_BIND(brls::ScrollingFrame, seasonFrame, "request/seasons/frame");
    std::vector<int> selectedSeasons;

    BRLS_BIND(brls::Button, requestButton, "request/button/approve");
    BRLS_BIND(brls::Button, cancelButton, "request/button/cancel");
    BRLS_BIND(brls::Box, content, "request/content");
    BRLS_BIND(brls::AppletFrame, applet, "request/applet");
};