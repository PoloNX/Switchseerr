#include "activity/MainActivity.hpp"
#include "tab/DiscoverTab.hpp"

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

    // Additional setup can be done here
}

MainActivity::~MainActivity() {
    brls::Logger::debug("MainActivity: destroy");
}