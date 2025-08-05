#pragma once

#include <borealis.hpp>
#include <memory>

#include "http/HttpClient.hpp"
#include "api/Jellyseerr.hpp"


class ConnectionCell : public brls::Box {
public:
    ConnectionCell() : serverType(jellyseerr::ConnectionServer::LOCAL) {
        this->inflateFromXMLRes("xml/view/connection_cell.xml");
        updateServerType();
    }

    ConnectionCell(jellyseerr::ConnectionServer serverType) : serverType(serverType) {
        this->inflateFromXMLRes("xml/view/connection_cell.xml");
        updateServerType();
    }

    void setServerType(jellyseerr::ConnectionServer newServerType) {
        this->serverType = newServerType;
        updateServerType();
    }

    static View* create() {
        return new ConnectionCell();
    }
    
    void updateServerType() {
        switch(serverType) {
            case jellyseerr::ConnectionServer::JELLYFIN:
                this->serverName->setText("Jellyfin");
                this->serverIcon->setImageFromRes("img/jellyfin.png");
                break;
            case jellyseerr::ConnectionServer::PLEX:
                this->serverName->setText("Plex");
                this->serverIcon->setImageFromRes("img/plex.png");
                break;
            case jellyseerr::ConnectionServer::LOCAL:
                this->serverName->setText("Jellyseerr");
                this->serverIcon->setImageFromRes("img/jellyseerr.png");
                break;
        }
    }

    BRLS_BIND(brls::Label, serverName, "server/name");
private:
    jellyseerr::ConnectionServer serverType;


    BRLS_BIND(brls::Image, serverIcon, "server/icon");
};

class ServerLogin : public brls::Box {
public:
    ServerLogin(std::shared_ptr<HttpClient> httpClient, const std::string& serverUrl, const std::string& user = "");
    ~ServerLogin();

    bool onSignin();
private:
    BRLS_BIND(brls::Header, headerSignin, "server/signin_to");
    BRLS_BIND(brls::Box, inputUser, "server/user");
    BRLS_BIND(brls::Box, inputPassword, "server/password");
    BRLS_BIND(brls::Button, buttonSignin, "server/signin");
    BRLS_BIND(brls::Image, imageServer, "server/image");
    BRLS_BIND(brls::Label, usernameLabel, "server/username_label");
    BRLS_BIND(brls::Label, passwordLabel, "server/password_label");
    BRLS_BIND(brls::Box, otherBox, "server/other");
    BRLS_BIND(ConnectionCell, connectionCell, "server/other/connection_cell");

    void handleServerInfoResponse(const jellyseerr::PublicServerInfo& serverInfo);
    void setupMediaServerLogin(jellyseerr::ConnectionServer serverType);
    void updateUIForServerType();
    void setupConnectionToggle();
    void toggleMediaServerConnection();

    std::string username;
    std::string password;

    // The server type is by default JELLYFIN, but can be set to PLEX
    jellyseerr::ConnectionServer serverType = jellyseerr::ConnectionServer::JELLYFIN;
    bool mediaServerLogin = false;

    std::string url;
    std::shared_ptr<HttpClient> httpClient;
};