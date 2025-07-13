#include "activity/MainActivity.hpp"

MainActivity::MainActivity(HttpClient& httpClient, AuthService& authService)
    : brls::Activity(), httpClient(httpClient), authService(authService) {
    brls::Logger::debug("MainActivity: create");


    // Initialize the tab frame or other components here if needed
}

void MainActivity::onContentAvailable() {
    brls::Logger::debug("MainActivity: content available");

    // You can set up your views or perform actions once the content is available
    this->appletFrame->setTitle("Main Activity");
    this->appletFrame->unregisterAction(brls::BUTTON_B);
}

MainActivity::~MainActivity() {
    brls::Logger::debug("MainActivity: destroy");
}