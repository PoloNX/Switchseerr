#pragma once

#include <borealis.hpp>

class AboutView : public brls::Box {
public:
    AboutView();
    
    void offsetTick();
    brls::Animatable showOffset = 0;
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;
    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;


    bool isTranslucent() override
    {
        return true;
    }
  
private:
    BRLS_BIND(brls::Box, ghRepoBox, "about/repoBox");
    BRLS_BIND(brls::Label, ghRepoLabel, "about/repoLink");
    BRLS_BIND(brls::Label, versionLabel, "about/version");
    BRLS_BIND(brls::Box, content, "about/content");
    BRLS_BIND(brls::AppletFrame, applet, "about/applet");

};