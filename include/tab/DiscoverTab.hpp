#pragma once

#include <borealis.hpp>

#include "view/RecyclingVideo.hpp"

class RecyclingVideo;

class DiscoverTab : public brls::Box {
public:
    DiscoverTab(HttpClient& httpClient, AuthService& authService);

    void willAppear(bool resetState = false) override;

private:
    HttpClient& httpClient;
    AuthService& authService;

    RecyclingVideo* latestMedias;

    BRLS_BIND(brls::Box, boxLatest, "discover/latest");
    //BRLS_BIND(RecyclingVideo, latestMedias, "discover/latest");
};