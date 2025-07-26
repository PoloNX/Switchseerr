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

    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("HRecyclerFrame", HRecyclerFrame::create);
    brls::Application::registerXMLView("LabelBackground", LabelBackground::create);

    brls::Theme::getLightTheme().addColor("color/grey_1", nvgRGB(245, 246, 247));
    brls::Theme::getDarkTheme().addColor("color/grey_1", nvgRGB(51, 52, 53));
    brls::Theme::getLightTheme().addColor("color/grey_2", nvgRGB(245, 245, 245));
    brls::Theme::getDarkTheme().addColor("color/grey_2", nvgRGB(51, 53, 55));
    brls::Theme::getLightTheme().addColor("color/grey_3", nvgRGBA(200, 200, 200, 16));
    brls::Theme::getDarkTheme().addColor("color/grey_3", nvgRGBA(160, 160, 160, 160));

    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);

    auto httpClient = std::make_shared<HttpClient>();

    auto serverList = new ServerList(httpClient);

    brls::Application::pushActivity(serverList, brls::TransitionAnimation::NONE);

    while(brls::Application::mainLoop()) ;

    ThreadPool::instance().stop();

    return 0;

}