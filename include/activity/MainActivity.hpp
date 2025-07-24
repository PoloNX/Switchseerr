#pragma once

#include <borealis.hpp>
#include <memory>

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"

class MainActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/main_activity.xml");

    MainActivity(std::shared_ptr<HttpClient> httpClient, AuthService& authService);
    ~MainActivity();

    void onContentAvailable() override;

private:
    std::shared_ptr<HttpClient> httpClient;
    AuthService& authService;

    BRLS_BIND(brls::TabFrame, tabFrame, "main/tab_frame");
    BRLS_BIND(brls::AppletFrame, appletFrame, "main/applet_frame");
};