#pragma once

#include <borealis.hpp>

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"

class MainActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/main_activity.xml");

    MainActivity(HttpClient& httpClient, AuthService& authService);
    ~MainActivity();

    void onContentAvailable() override;

private:
    HttpClient& httpClient;
    AuthService& authService;

    BRLS_BIND(brls::AppletFrame, appletFrame, "main/applet_frame");
};