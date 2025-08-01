#include "activity/MainActivity.hpp"
#include "tab/DiscoverTab.hpp"
#include "tab/MediaTab.hpp"
#include "view/SvgImage.hpp"
#include "tab/SearchTab.hpp"

MainActivity::MainActivity(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : httpClient(httpClient), authService(authService) {
    brls::Logger::debug("MainActivity: create");
}

void MainActivity::onContentAvailable() {
    brls::Logger::debug("MainActivity: content available");

    // Créer un onglet Discover
    auto* discoverTab = new AutoSidebarItem();
    discoverTab->setTabStyle(AutoTabBarStyle::ACCENT);
    discoverTab->setSVGIcon(std::string(BRLS_RESOURCES) + "icon/icon-discover.svg");
    discoverTab->setSVGActivateIcon(std::string(BRLS_RESOURCES) + "icon/icon-discover-selected.svg");
    discoverTab->setFontSize(18);
    tabFrame->addTab(discoverTab, [this]() -> brls::View* {
        auto container = new DiscoverTab(httpClient, authService);
        return container;
    });

    auto* movieTab = new AutoSidebarItem();
    movieTab->setTabStyle(AutoTabBarStyle::ACCENT);
    movieTab->setSVGIcon(std::string(BRLS_RESOURCES) + "icon/icon-movie.svg");
    movieTab->setSVGActivateIcon(std::string(BRLS_RESOURCES) + "icon/icon-movie-selected.svg");
    tabFrame->addTab(movieTab, [this]() -> brls::View* {
        return new MediaTab(httpClient, authService, MediaType::Movie);
    });

    auto* tvTab = new AutoSidebarItem();
    tvTab->setTabStyle(AutoTabBarStyle::ACCENT);
    tvTab->setSVGIcon(std::string(BRLS_RESOURCES) + "icon/icon-tv.svg");
    tvTab->setSVGActivateIcon(std::string(BRLS_RESOURCES) + "icon/icon-tv-selected.svg");
    tabFrame->addTab(tvTab, [this]() -> brls::View* {
        return new MediaTab(httpClient, authService, MediaType::Tv);
    });

    auto* searchTab = new AutoSidebarItem();
    searchTab->setTabStyle(AutoTabBarStyle::ACCENT);
    searchTab->setSVGIcon(std::string(BRLS_RESOURCES) + "icon/icon-search.svg");
    searchTab->setSVGActivateIcon(std::string(BRLS_RESOURCES) + "icon/icon-search-selected.svg");
    tabFrame->addTab(searchTab, [this]() -> brls::View* {
        return new SearchTab(httpClient, authService);
    });
}

MainActivity::~MainActivity() {
    brls::Logger::debug("MainActivity: destroy");
}