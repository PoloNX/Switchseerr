#pragma once

#include <borealis.hpp>

#include "models/MediaItem.hpp"

class MediaInfoView : public brls::Box {
public:
    MediaInfoView(MediaItem mediaItem);

    void offsetTick();
    brls::Animatable showOffset = 0;
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;
    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;
    bool isTranslucent() override
    {
        return true;
    }

private:
    void setupMediaInfo();
    void addHeader(const std::string& title, const std::string& subtitle);

    MediaItem mediaItem;

    BRLS_BIND(brls::Box, contentBox, "info/content");
    BRLS_BIND(brls::Box, detailsBox, "info/content/details");
    BRLS_BIND(brls::AppletFrame, applet, "info/applet");
    BRLS_BIND(brls::Button, backButton, "info/back_button");
};