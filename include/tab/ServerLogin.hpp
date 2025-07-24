#pragma once

#include <borealis.hpp>
#include <memory>

#include "http/HttpClient.hpp"

class ServerLogin : public brls::Box {
public:
    ServerLogin(std::shared_ptr<HttpClient> httpClient, const std::string& serverUrl, const std::string& user = "");
    ~ServerLogin();

    bool onSignin();
private:
    BRLS_BIND(brls::Header, headerSignin, "server/signin_to");
    BRLS_BIND(brls::InputCell, inputUser, "server/user");
    BRLS_BIND(brls::InputCell, inputPassword, "server/password");
    BRLS_BIND(brls::Button, buttonSignin, "server/signin");
    BRLS_BIND(brls::Image, imageServer, "server/image");
    
    std::string url;
    std::shared_ptr<HttpClient> httpClient;
};