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
    std::vector<QualityProfile> availableQualityProfiles;

    BRLS_BIND(brls::Box, shadowImage, "request/image/background");
    BRLS_BIND(brls::Image, backdropImage, "request/image");
    BRLS_BIND(brls::Label, titleLabel, "request/title");
    BRLS_BIND(brls::Label, mediaLabel, "request/media");
    BRLS_BIND(brls::DetailCell, qualityCell, "request/quality");
    BRLS_BIND(brls::Button, requestButton, "request/button/approve");
    BRLS_BIND(brls::Button, cancelButton, "request/button/cancel");
    BRLS_BIND(brls::Box, content, "request/content");
    BRLS_BIND(brls::AppletFrame, applet, "request/applet");
};