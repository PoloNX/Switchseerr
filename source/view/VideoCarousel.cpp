#include "view/VideoCarousel.hpp"
#include "api/Jellyseerr.hpp"
#include "view/MediaPreview.hpp"
#include "utils/ThreadPool.hpp"

static std::mutex downloadMutex;

VideoCarousel::VideoCarousel(std::shared_ptr<HttpClient> httpClient, AuthService& authService, DiscoverType type)
    : httpClient(httpClient), authService(authService), type(type) {
    brls::Logger::debug("VideoCarousel: Creating carousel for type {}", static_cast<int>(type));
    this->inflateFromXMLRes("xml/view/video_carousel.xml");
    this->scrollingFrame->setFocusable(true);
    this->setMargins(10, 0, 10, 0);

    serverUrl = authService.getServerUrl();
    apiKey = authService.getToken().value_or("");
}

VideoCarousel::~VideoCarousel() {
    brls::Logger::debug("VideoCarousel: Destroying carousel for type {}", static_cast<int>(type));
}

void VideoCarousel::doRequest() {
    brls::Logger::debug("VideoCarousel: Requesting data for type {}", static_cast<int>(type));

    // Clean up previous carousel items
    carouselBox->clearViews();

    
    brls::async([this]() {
        // Configure le titre et récupère les données
        configureHeaderTitle();
        auto items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, type, 20);

        // Crée les cartes pour chaque item
        for (auto& item : items) {
            ASYNC_RETAIN
            brls::sync([ASYNC_TOKEN, item]() mutable {
                ASYNC_RELEASE
                createAndAddVideoCard(item);
            });
        }
        });
}

void VideoCarousel::configureHeaderTitle() {
    static const std::unordered_map<DiscoverType, std::string> titleMap = {
        {DiscoverType::RecentlyAdded, "Recently Added"},
        {DiscoverType::Trending, "Trending"},
        {DiscoverType::PopularMovies, "Popular Movies"},
        {DiscoverType::PopularTvShows, "Popular TV Shows"},
        {DiscoverType::FutureMovies, "Future Movies"},
        {DiscoverType::FutureTvShows, "Future TV Shows"}
    };

    if (auto it = titleMap.find(type); it != titleMap.end()) {
        header->setTitle(it->second);
    }
}

void VideoCarousel::createAndAddVideoCard(MediaItem& item) {
    auto videoCard = new VideoCardCell();

    // Configuration de base
    setupVideoCardContent(videoCard, item);
    setupVideoCardStyling(videoCard, item);
    setupVideoCardInteractions(videoCard, item);

    // Chargement asynchrone de l'image
    loadVideoCardImage(videoCard, item);

    // Ajout à l'interface
    addVideoCardToCarousel(videoCard);
}

void VideoCarousel::setupVideoCardContent(VideoCardCell* videoCard, const MediaItem& item) {
    videoCard->labelTitle->setText(item.title);
    videoCard->labelExt->setText(item.releaseDate.empty() ? item.firstAirDate : item.releaseDate);
    videoCard->labelRating->setText(std::to_string(item.voteAverage));
}

void VideoCarousel::setupVideoCardStyling(VideoCardCell* videoCard, const MediaItem& item) {
    switch(item.type) {
        case MediaType::Movie:
            videoCard->labelBackground->setStyle(LabelBackgroundStyle::Movie);
            videoCard->labelBackground->setText("Film");
            break;
        case MediaType::Tv:
            videoCard->labelBackground->setStyle(LabelBackgroundStyle::TVShow);
            videoCard->labelBackground->setText("Série");
            break;

    }
    videoCard->setMarginLeft(5);
    videoCard->setMarginRight(5);
}

void VideoCarousel::setupVideoCardInteractions(VideoCardCell* videoCard, MediaItem& item) {
    videoCard->registerClickAction([this, item](brls::View* view) mutable {
        brls::Logger::debug("VideoCarousel: Item clicked with ID: {}", item.id);
        auto mediaPreview = new MediaPreview(httpClient, authService, item);
        this->present(mediaPreview);
        getAppletFrame()->setHeaderVisibility(brls::Visibility::GONE);
        return true;
    });
}

void VideoCarousel::loadVideoCardImage(VideoCardCell* videoCard, const MediaItem& item) {
    if (item.posterPath.empty()) return;

    auto& threadPool = ThreadPool::instance();

    threadPool.submit([this, item, videoCard](std::shared_ptr<HttpClient> client) {
        brls::Logger::debug("VideoCarousel: Downloading image for item: {}", item.title);

        auto imageBuffer = client->downloadImageToBuffer(
            fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", item.posterPath)
        );

        if (!imageBuffer.empty()) {
            ASYNC_RETAIN
            brls::sync([ASYNC_TOKEN, videoCard, item, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                    if (videoCard && videoCard->picture) {
                        try {
                            brls::Logger::debug("VideoCarousel: Image loaded for item: {}", item.id);
                            videoCard->picture->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                        }
                        catch (const std::exception& e) {
                            brls::Logger::error("VideoCarousel: Error setting image: {}", e.what());
                        }
                    }
                    else {
                        brls::Logger::debug("VideoCarousel: VideoCard destroyed, skipping image");
                    }
                });
        }
        else {
            brls::Logger::error("VideoCarousel: Failed to download image for item: {}", item.id);
        }
    });
}

void VideoCarousel::addVideoCardToCarousel(VideoCardCell* videoCard) {
    if (!carouselBox) {
        brls::Logger::error("VideoCarousel: carouselBox is null");
        return;
    }

    try {
        carouselBox->addView(videoCard);
    }
    catch (const std::exception& e) {
        brls::Logger::error("VideoCarousel: Exception adding video card: {}", e.what());
    }
}