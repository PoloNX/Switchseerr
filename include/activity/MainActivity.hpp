#pragma once

#include <borealis.hpp>
#include <memory>

#include "view/AutoTabFrame.hpp"
#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"

class MainActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/main_activity.xml");

    MainActivity(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService);
    ~MainActivity();

    void onContentAvailable() override;

private:
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;


    BRLS_BIND(AutoTabFrame, tabFrame, "main/tab_frame");
    BRLS_BIND(brls::AppletFrame, appletFrame, "main/applet_frame");
};