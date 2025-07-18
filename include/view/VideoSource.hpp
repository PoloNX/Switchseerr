#pragma once

#include "view/RecyclingGrid.hpp"
#include "models/MediaItem.hpp"
#include "http/HttpClient.hpp"

#include <variant>

class VideoDataSource : public RecyclingGridDataSource {
public:
    using MediaList = std::vector<MediaItem>;

    explicit VideoDataSource(const MediaList& r, HttpClient& httpClient, bool resume = false);

    size_t getItemCount() override;

    RecyclingGridItem* cellForRow(RecyclingView* recycler, size_t index) override;

    void onItemSelected(brls::Box* recycler, size_t index) override;

    void clearData() override;

    void appendData(const MediaList& data);

protected:
    HttpClient& httpClient;
    MediaList list;
    bool resume;

    // Ajout des variables pour ASYNC_RETAIN
    bool* deletionToken = nullptr;
    int* deletionTokenCounter = nullptr;
};