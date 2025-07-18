#include "activity/MainActivity.hpp"
#include "tab/DiscoverTab.hpp"

MainActivity::MainActivity(HttpClient& httpClient, AuthService& authService)
    :  httpClient(httpClient), authService(authService) {
    brls::Logger::debug("MainActivity: create");

    // Initialize the tab frame or other components here if needed
}

void MainActivity::onContentAvailable() {
    brls::Logger::debug("MainActivity: content available");
    
    tabFrame->addTab("Discover", [this]() {
        return new DiscoverTab(httpClient, authService);
    });

    // Additional setup can be done here
}

MainActivity::~MainActivity() {
    brls::Logger::debug("MainActivity: destroy");
}