/*
    Copyright 2025 dragonflylee
*/

#pragma once

#include <borealis.hpp>

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"

class HRecyclerFrame;

class RecyclingVideo : public brls::Box {
public:
    RecyclingVideo(HttpClient& httpClient, AuthService& authService);
    ~RecyclingVideo() override;

    static brls::View* create();

    using Callback = std::function<std::string(size_t, size_t)>;

    void reset() { this->start = 0; }
    void setTitle(const std::string& text);
    void onQuery(const Callback& callback = nullptr);
    void doRequest(bool refresh = false);
    void doLatest(bool refresh = false);


    Callback queryCallback = nullptr;
    bool resume = false;
    size_t start = 0;
    size_t pageSize = 10;

    BRLS_BIND(HRecyclerFrame, recycler, "recycler/videos");

private:
    BRLS_BIND(brls::Header, title, "recycler/title");


    HttpClient& httpClient;
    AuthService& authService;

};