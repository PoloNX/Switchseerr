#pragma once

#include <borealis.hpp>
#include <memory>

#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"

class DiscoverTab : public brls::Box {
public:
    DiscoverTab(std::shared_ptr<HttpClient> httpClient, AuthService& authService);
    ~DiscoverTab();

private:
    std::shared_ptr<HttpClient> httpClient;
    AuthService& authService;

    BRLS_BIND(brls::Box, boxLatest, "discover/latest");
};