#include "view/MediaGridView.hpp"
#include "view/MediaPreview.hpp"
#include "utils/ThreadPool.hpp"
#include "api/Jellyseerr.hpp"

MediaGridData::MediaGridData(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type)
    : httpClient(httpClient), authService(authService), mediaType(type) {
    this->serverUrl = authService->getServerUrl();
    this->apiKey = authService->getToken().value_or("");
}

bool MediaGridData::loadData() {
    auto medias = jellyseerr::getMedias(this->httpClient, this->serverUrl, this->apiKey, mediaType);
    if (!medias.empty()) {
        this->items = medias;
    }
    return !items.empty();
}

RecyclingGridItem* MediaGridData::cellForRow(RecyclingView* recycler, size_t index) {
    auto cell = (VideoCardCell*)recycler->dequeueReusableCell("Cell");
    brls::Logger::debug("MediaGridData::cellForRow index: {}", items[index].title);
    cell->labelTitle->setText(items[index].title);
    cell->labelExt->setText(items[index].releaseDate.empty() ? items[index].firstAirDate : items[index].releaseDate);
    cell->labelRating->setText(std::to_string(items[index].voteAverage));
    return cell;    
}

size_t MediaGridData::getItemCount() {
    return items.size();
}

void MediaGridData::onItemSelected(brls::Box* recycler, size_t index) {
    brls::Logger::debug("MediaGridData::onItemSelected index: {}", index);
    recycler->present(new MediaPreview(httpClient, authService, items[index], recycler));
}

void MediaGridData::clearData() {
    items.clear();
}


MediaGridView::MediaGridView(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type)
    : httpClient(httpClient), authService(authService), mediaType(type) {
    this->inflateFromXMLRes("xml/view/media_grid.xml");
    data = std::make_unique<MediaGridData>(httpClient, authService, type);
    recycler->setDataSource(data.get());
    recycler->estimatedRowHeight = 240;
    

    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        data->loadData();
        brls::sync([ASYNC_TOKEN]() {
            recycler->reloadData();
        });
    });
}

MediaGridView::~MediaGridView() {
    // Destructor logic if needed
}