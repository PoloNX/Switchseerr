#pragma once

#include <borealis.hpp>

#include "http/HttpClient.hpp"

class ServerAdd : public brls::Box {
public:
    ServerAdd(HttpClient& httpClient);
    ~ServerAdd() override;

    brls::View* getDefaultFocus() override;
private:
    bool onConnect();

    HttpClient& httpClient;

    BRLS_BIND(brls::Image, jellyseerrImage, "server/image");
    BRLS_BIND(brls::InputCell, inputUrl, "server/url");
    BRLS_BIND(brls::Button, connectButton, "server/connect");
};