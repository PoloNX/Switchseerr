#include <iostream>
#include <memory>

#include "activity/ServerList.hpp"
#include "http/HttpClient.hpp"
#include "utils/Config.hpp"
#include "view/RecyclingGrid.hpp"
#include "view/HRecycling.hpp"
#include "tab/DiscoverTab.hpp"
#include "utils/ThreadPool.hpp"
#include "view/LabelBackground.hpp"
#include "view/AutoTabFrame.hpp"
#include "view/SvgImage.hpp"

#include <borealis.hpp>
#include <memory>

int main() {
    brls::Platform::APP_LOCALE_DEFAULT = brls::LOCALE_AUTO;

    if (!brls::Application::init()) {
        brls::Logger::error("Failed to initialize Borealis application.");
        return 1;
    }

    brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);

    auto& conf = Config::instance();
    if(!conf.init()) {
        brls::Logger::error("Failed to initialize configuration.");
        return 1;
    }

    brls::Application::createWindow("Switchseerr");
    brls::Application::setGlobalQuit(false);
    brls::Application::getPlatform()->getInputManager()->getKeyboardKeyStateChanged()->subscribe(
        [](brls::KeyState state) {
            if(!state.pressed) return;
            auto top = brls::Application::getActivitiesStack().back();
            switch(state.key) {
                case brls::BRLS_KBD_KEY_F:
#ifndef __APPLE__
                case brls::BRLS_KBD_KEY_F11:
#endif
                    brls::Application::getPlatform()->getVideoContext()->fullScreen(!VideoContext::FULLSCREEN);
                    break;
            }
        }
    );

    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("HRecyclerFrame", HRecyclerFrame::create);
    brls::Application::registerXMLView("LabelBackground", LabelBackground::create);
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);

    brls::Theme::getLightTheme().addColor("color/app", nvgRGB(2, 176, 183));
    brls::Theme::getDarkTheme().addColor("color/app", nvgRGB(51, 186, 227));

    brls::Theme::getDarkTheme().addColor("color/background_start", nvgRGBA(45, 45, 45, 150));
    brls::Theme::getDarkTheme().addColor("color/background_end", nvgRGBA(45, 45, 45, 255));

    brls::Theme::getLightTheme().addColor("color/grey_1", nvgRGB(245, 246, 247));
    brls::Theme::getDarkTheme().addColor("color/grey_1", nvgRGB(51, 52, 53));
    brls::Theme::getLightTheme().addColor("color/grey_2", nvgRGB(245, 245, 245));
    brls::Theme::getDarkTheme().addColor("color/grey_2", nvgRGB(51, 53, 55));
    brls::Theme::getLightTheme().addColor("color/grey_3", nvgRGBA(200, 200, 200, 16));
    brls::Theme::getDarkTheme().addColor("color/grey_3", nvgRGBA(160, 160, 160, 160));

    brls::Theme::getLightTheme().addColor("font/grey", nvgRGB(148, 153, 160));
    brls::Theme::getDarkTheme().addColor("font/grey", nvgRGB(148, 153, 160));

    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);

    auto httpClient = std::make_shared<HttpClient>();

    auto serverList = new ServerList(httpClient);

    brls::Application::pushActivity(serverList, brls::TransitionAnimation::NONE);

    while(brls::Application::mainLoop()) ;

    ThreadPool::instance().stop();

    return 0;

}