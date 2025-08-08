#include "view/VideoCarousel.hpp"
#include "api/Jellyseerr.hpp"
#include "view/MoviePreview.hpp"
#include "view/TvPreview.hpp"
#include "utils/ThreadPool.hpp"

using namespace brls::literals;

static std::mutex downloadMutex;

VideoCarousel::VideoCarousel(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, DiscoverType type)
    : httpClient(httpClient), authService(authService), type(type) {
    brls::Logger::debug("VideoCarousel: Creating carousel for type {}", static_cast<int>(type));
    this->inflateFromXMLRes("xml/view/video_carousel.xml");
    this->setMargins(10, 0, 10, 0);

    serverUrl = authService->getServerUrl();
}

VideoCarousel::~VideoCarousel() {
    brls::Logger::debug("VideoCarousel: Destroying carousel for type {}", static_cast<int>(type));
}

void VideoCarousel::doRequest() {
    brls::Logger::debug("VideoCarousel: Requesting data for type {}", static_cast<int>(type));

    // Clean up previous carousel items
    carouselBox->clearViews();

    ASYNC_RETAIN
    brls::async([ASYNC_TOKEN]() {
        // Configure le titre et récupère les données
        configureHeaderTitle();
        auto items = jellyseerr::getMedias(httpClient, serverUrl, type, 20);

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
        {DiscoverType::RecentlyAdded, "main/view/carousel/recently_added"_i18n},
        {DiscoverType::Trending, "main/view/carousel/trending"_i18n},
        {DiscoverType::PopularMovies, "main/view/carousel/popular_movies"_i18n},
        {DiscoverType::PopularTvShows, "main/view/carousel/popular_tv_shows"_i18n},
        {DiscoverType::FutureMovies, "main/view/carousel/future_movies"_i18n},
        {DiscoverType::FutureTvShows, "main/view/carousel/future_tv_shows"_i18n}
    };

    if (auto it = titleMap.find(type); it != titleMap.end()) {
        header->setTitle(it->second);
        header->setTitleSize(20);
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
    videoCard->setStatus(item.status);
    // Format the rating to two decimal places
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << item.voteAverage;
    if (ss.str() == "0.0") {
        ss.str("N/A"); // If the rating is 0.0, display "N/A"
    }
    videoCard->labelRating->setText(ss.str());
}

void VideoCarousel::setupVideoCardStyling(VideoCardCell* videoCard, const MediaItem& item) {
    switch(item.type) {
        case MediaType::Movie:
            videoCard->labelBackground->setStyle(LabelBackgroundStyle::Movie);
            videoCard->labelBackground->setText("main/models/movie"_i18n);
            break;
        case MediaType::Tv:
            videoCard->labelBackground->setStyle(LabelBackgroundStyle::TVShow);
            videoCard->labelBackground->setText("main/models/tv_show"_i18n);
            break;

    }
    videoCard->setMarginLeft(5);
    videoCard->setMarginRight(5);
}

void VideoCarousel::setupVideoCardInteractions(VideoCardCell* videoCard, MediaItem& item) {
    videoCard->registerClickAction([this, item](brls::View* view) mutable {
        brls::Logger::debug("VideoCarousel: Item clicked with ID: {}", item.id);
        if (item.type == MediaType::Movie) {
            brls::Logger::debug("VideoCarousel: Opening MoviePreview for item ID: {}", item.id);
            auto moviePreview = new MoviePreview(httpClient, authService, item, this);
            this->present(moviePreview);
        } else {
            auto tvPreview = new TvPreview(httpClient, authService, item, this);
            brls::Logger::debug("VideoCarousel: Opening TvPreview for item ID: {}", item.id);
            this->present(tvPreview);
        }
        return true;
    });
}

void VideoCarousel::loadVideoCardImage(VideoCardCell* videoCard, const MediaItem& item) {
    if (item.posterPath.empty()) return;

    auto& threadPool = ThreadPool::instance();

    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN, item, videoCard](std::shared_ptr<HttpClient> client) {
        brls::Logger::verbose("VideoCarousel: Downloading image for item: {}", item.title);

        auto imageBuffer = client->downloadImageToBuffer(
            fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", item.posterPath)
        );

        if (!imageBuffer.empty()) {
            brls::sync([ASYNC_TOKEN, videoCard, item, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                    if (videoCard && videoCard->picture) {
                        try {
                            brls::Logger::verbose("VideoCarousel: Image loaded for item: {}", item.id);
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