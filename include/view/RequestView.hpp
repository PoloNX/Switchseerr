#pragma once

#include "models/MediaItem.hpp"

#include <borealis.hpp>

class RequestView : public brls::Box {
public:
    RequestView(MediaItem mediaItem);

    void offsetTick();
    brls::Animatable showOffset = 0;
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;
    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;

    bool isTranslucent() override
    {
        return true;
    }
private:
    MediaItem mediaItem;

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