#pragma once

#include <borealis.hpp>

#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "view/RecyclingGrid.hpp"
#include "view/MediaGridView.hpp"
#include "view/SvgImage.hpp"

class SearchGridView : public brls::Box {
public:
    SearchGridView(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService);
    ~SearchGridView() override;

private:
    MediaType mediaType;
    std::unique_ptr<MediaGridData> data;
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;

    int currentPage = 1;
    std::string currentSearchQuery;

    void updateData();
    void onPreviousPage();
    void onNextPage();

    BRLS_BIND(SVGImage, searchIcon, "media/search/svg");
    BRLS_BIND(brls::Label, inputLabel, "media/search/input");
    BRLS_BIND(brls::Box, searchBox, "media/search");
    BRLS_BIND(brls::Label, pageIndicator, "media/page/label");
    BRLS_BIND(brls::Button, nextPageButton, "media/page/next");
    BRLS_BIND(brls::Button, previousPageButton, "media/page/prev");
    BRLS_BIND(brls::Box, pageBox, "media/page");
    BRLS_BIND(RecyclingGrid, recycler, "media/recycler");
};