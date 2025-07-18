#pragma once

#include "http/HttpClient.hpp"
#include "utils/Config.hpp"
#include "view/RecyclingGrid.hpp"

#include <borealis.hpp>

class ServerList : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/server_list.xml");

    ServerList(HttpClient& httpClient);
    ~ServerList();

    void onUser(const std::string& id);
    std::string getUrl();

    HttpClient& getHttpClient() {
        return this->httpClient;
    }

    void willAppear(bool resetState = false) override;
    void onContentAvailable() override;
private:

    HttpClient& httpClient;

    BRLS_BIND(brls::Button, btnServerAdd, "btn/server/add");
    BRLS_BIND(brls::Box, sidebarServers, "server/sidebar");
    BRLS_BIND(brls::Box, serverDetail, "server/detail");
    BRLS_BIND(RecyclingGrid, recyclerUsers, "user/recycler");
    BRLS_BIND(brls::DetailCell, serverVersion, "server/version");
    BRLS_BIND(brls::DetailCell, inputUrl, "selector/server/urls");
    BRLS_BIND(brls::Button, btnSignin, "btn/server/signin");
    BRLS_BIND(brls::AppletFrame, appletFrame, "server/frame");

    void onServer(const AppServer &s);
    void setActive(brls::View *active);
    void getActive();
};

class ServerCell : public brls::Box {
public:
    ServerCell(const AppServer& server);

    void setActive(bool active);
    bool getActive();
private:
    BRLS_BIND(brls::Rectangle, accent, "brls/sidebar/item_accent");
    BRLS_BIND(brls::Label, url, "server/url");
};
