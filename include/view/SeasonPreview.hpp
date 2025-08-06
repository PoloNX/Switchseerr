#pragma once

#include <borealis.hpp>
#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"


class SeasonPreview : public brls::Box {
public:
    SeasonPreview(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, Season& season);
    
    // Override the show and hide methods to handle animations
    void offsetTick();
    brls::Animatable showOffset = 0;
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;
    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;

    bool isTranslucent() override
    {
        return true;
    }

private:
    Season season;
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;

    void loadSeasonDetails();

    BRLS_BIND(brls::AppletFrame, applet, "season/applet");
    BRLS_BIND(brls::Box, content, "season/content");
    BRLS_BIND(brls::Label, titleLabel, "season/title");
    BRLS_BIND(brls::Box, episodesBox, "season/episodes");
    BRLS_BIND(brls::Button, backButton, "season/back_button");
    BRLS_BIND(brls::ScrollingFrame, frame, "season/frame");
};