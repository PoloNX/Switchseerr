#include "activity/MainActivity.hpp"
#include "tab/DiscoverTab.hpp"
#include "tab/MovieTab.hpp"

MainActivity::MainActivity(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : httpClient(httpClient), authService(authService) {
    brls::Logger::debug("MainActivity: create");
}

void MainActivity::onContentAvailable() {
    brls::Logger::debug("MainActivity: content available");
    
    tabFrame->setSidebarWidth(300);
    tabFrame->addTab("Discover", [this]() {
        return new DiscoverTab(httpClient, authService); // Pass httpClient as shared_ptr if DiscoverTab expects it
    });

    tabFrame->addTab("Movies", [this]() {
        return new MovieTab(httpClient, authService);
    });

    // Additional setup can be done here
}

MainActivity::~MainActivity() {
    brls::Logger::debug("MainActivity: destroy");
}