#pragma once

#include <borealis.hpp>

#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"

class DiscoverTab : public brls::Box {
public:
    DiscoverTab(HttpClient& httpClient, AuthService& authService);

    void willAppear(bool resetState = false) override;

private:
    HttpClient& httpClient;
    AuthService& authService;

    BRLS_BIND(brls::Box, boxLatest, "discover/latest");
};