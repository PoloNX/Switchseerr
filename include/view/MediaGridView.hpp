#pragma once

#include <borealis.hpp>

#include "view/RecyclingGrid.hpp"
#include "view/VideoCard.hpp"
#include "auth/AuthService.hpp"
#include "http/HttpClient.hpp"
#include "models/MediaItem.hpp"

class MediaGridData : public RecyclingGridDataSource {
public:
    MediaGridData(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type);

    RecyclingGridItem* cellForRow(RecyclingView* recycler, size_t index) override;
    size_t getItemCount() override;
    void onItemSelected(brls::Box* recycler, size_t index) override;
    void clearData() override;
    bool loadData();
    void setItems(std::vector<MediaItem>&& newItems);
    
private:
    std::string serverUrl;
    std::string apiKey;
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;
    MediaType mediaType;
    std::vector<MediaItem> items;
};

class MediaGridView : public brls::Box {
public:
    MediaGridView(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type);
    ~MediaGridView() override;

private:
    MediaType mediaType;
    std::unique_ptr<MediaGridData> data;
    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;

    BRLS_BIND(RecyclingGrid, recycler, "media/recycler");
};