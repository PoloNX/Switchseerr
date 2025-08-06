#include "view/MediaGridView.hpp"
#include "view/MoviePreview.hpp"
#include "view/TvPreview.hpp"
#include "utils/ThreadPool.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/utils.hpp"

using namespace brls::literals;

MediaGridData::MediaGridData(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type)
    : httpClient(httpClient), authService(authService), mediaType(type) {
    this->serverUrl = authService->getServerUrl();
}

bool MediaGridData::loadData(int page) {
    std::vector<MediaItem> medias = {};
    if (searchQuery.empty()) {
        medias = jellyseerr::getMedias(this->httpClient, this->serverUrl, mediaType, page);
    } else {
        medias = jellyseerr::searchMedias(this->httpClient, this->serverUrl, escaped_string(this->searchQuery), page);
    }
    if (!medias.empty()) {
        this->items = medias;
    }
    return !items.empty();
}

RecyclingGridItem* MediaGridData::cellForRow(RecyclingView* recycler, size_t index) {
    auto cell = (VideoCardCell*)recycler->dequeueReusableCell("Cell");
    brls::Logger::verbose("MediaGridData::cellForRow index: {}", items[index].title);
    cell->labelTitle->setText(items[index].title);
    cell->labelExt->setText(items[index].releaseDate.empty() ? items[index].firstAirDate : items[index].releaseDate);
    cell->setStatus(items[index].status);
        // Format the rating to two decimal places
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << items[index].voteAverage;
    cell->labelRating->setText(ss.str());

    switch(items[index].type) {
    case MediaType::Movie:
        cell->labelBackground->setStyle(LabelBackgroundStyle::Movie);
        cell->labelBackground->setText("main/models/movie"_i18n);
        break;
    case MediaType::Tv:
        cell->labelBackground->setStyle(LabelBackgroundStyle::TVShow);
        cell->labelBackground->setText("main/models/tv_show"_i18n);
        break;
    }

    //Image loading
    if (items[index].posterPath.empty()) {
        return cell;
    } 
    auto& threadPool = ThreadPool::instance();

    threadPool.submit([this, cell, index](std::shared_ptr<HttpClient> client) {
        brls::Logger::verbose("MediaGridData: Downloading image for item: {}", items[index].title);
        auto imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", items[index].posterPath));
        if (!imageBuffer.empty()) {
            brls::sync([cell, imageBuffer = std::move(imageBuffer), index, this]() {
                try {
                    brls::Logger::verbose("MediaGridData: Image loaded for item: {}", items[index].title);
                    cell->picture->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                } catch (const std::exception& e) {
                    brls::Logger::error("MediaGridData: Error setting image: {}", e.what());
                }
            });
        } else {
            brls::Logger::error("MediaGridData: Failed to download image for item: {}", items[index].title);
        }
    });

    return cell;    
}

size_t MediaGridData::getItemCount() {
    return items.size();
}

void MediaGridData::onItemSelected(brls::Box* recycler, size_t index) {
    brls::Logger::debug("MediaGridData::onItemSelected index: {}", index);
    if (items[index].type== MediaType::Movie) {
        recycler->present(new MoviePreview(httpClient, authService, items[index], recycler));
    } else {
        recycler->present(new TvPreview(httpClient, authService, items[index], recycler));
    }
    
}

void MediaGridData::clearData() {
    items.clear();
}


MediaGridView::MediaGridView(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType type)
    : httpClient(httpClient), authService(authService), mediaType(type) {
    this->inflateFromXMLRes("xml/view/media_grid.xml");
    data = std::make_unique<MediaGridData>(httpClient, authService, type);
    recycler->setDataSource(data.get());
    recycler->registerCell("Cell", VideoCardCell::create);
    recycler->estimatedRowSpace = 28;


    ThreadPool& threadPool = ThreadPool::instance();

    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        data->loadData();
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            brls::Logger::debug("MediaGridView::loadData completed");
            recycler->notifyDataChanged();
            recycler->reloadData();
        });
    });

    pageIndicator->setText(fmt::format("{} {}", "main/view/media_grid/page"_i18n ,currentPage));

    previousPageButton->registerClickAction([this](brls::View* view) {
        onPreviousPage();
        return true;
    });
    nextPageButton->registerClickAction([this](brls::View* view) {
        onNextPage();
        return true;
    });
}

void MediaGridView::onPreviousPage() {
    if (currentPage > 1) {
        currentPage--;
        brls::Logger::debug("MediaGridView: Loading previous page: {}", currentPage);
        ThreadPool& threadPool = ThreadPool::instance();
        ASYNC_RETAIN
        threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
            data->loadData(currentPage);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                brls::Logger::debug("MediaGridView::loadData completed");
                recycler->notifyDataChanged();
                recycler->reloadData();
                pageIndicator->setText(fmt::format("{} {}", "main/view/media_grid/page"_i18n, currentPage));
            });
        });
        recycler->notifyDataChanged();
    }
}
void MediaGridView::onNextPage() {
    currentPage++;
    brls::Logger::debug("MediaGridView: Loading next page: {}", currentPage);
    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        data->loadData(currentPage);
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            brls::Logger::debug("MediaGridView::loadData completed");
            recycler->notifyDataChanged();
            recycler->reloadData();
            pageIndicator->setText(fmt::format("{} {}", "main/view/media_grid/page"_i18n, currentPage));
        });
    });
    recycler->notifyDataChanged();
}

MediaGridView::~MediaGridView() {
    // Destructor logic if needed
}
