#include "view/MediaPreview.hpp"
#include "utils/ThreadPool.hpp"

MediaPreview::MediaPreview(std::shared_ptr<HttpClient> httpClient, AuthService& authService, MediaItem& mediaItem) : httpClient(httpClient), authService(authService), mediaItem(mediaItem) {
    brls::Logger::debug("MediaPreview: create");
    this->inflateFromXMLRes("xml/view/media_preview.xml");

    this->registerAction("back", brls::BUTTON_B, [this](brls::View* view) {
        brls::Logger::debug("MediaPreview: Back action triggered");
        getAppletFrame()->setHeaderVisibility(brls::Visibility::VISIBLE);
        this->dismiss();
        return true;
    });

    this->scroller->setFocusable(true);

    this->titleLabel->setText(mediaItem.title);
    this->overviewLabel->setText(mediaItem.overview);
    
    switch(mediaItem.status) {
        case MediaStatus::Pending:
            this->statusLabel->setText("Pending");
            break;
        case MediaStatus::Processing:
            this->statusLabel->setText("Processing");
            break;
        case MediaStatus::PartiallyAvailable:
            this->statusLabel->setText("Partially Available");
            break;
        case MediaStatus::Available:
            this->statusLabel->setText("Available");
            break;
        case MediaStatus::Blacklisted:
            this->statusLabel->setText("Blacklisted");
            break;
        case MediaStatus::Deleted:
            this->statusLabel->setText("Deleted");
            break;
        default:
            this->statusLabel->setText("Unknown Status");
    }
    
    downloadPosterImage();
    downloadBackdropImage();
}

void MediaPreview::downloadPosterImage() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", mediaItem.posterPath), true);
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                this->posterImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("MediaPreview, Failed to download poster image for item ID: {}", mediaItem.id);
        }
    });
}

void MediaPreview::downloadBackdropImage() {
    auto& threadPool = ThreadPool::instance();

    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = httpClient->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1280_and_h720_face{}", mediaItem.backdropPath), true);
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                this->backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("MediaPreview, Failed to download backdrop image for item ID: {}", mediaItem.id);
        }
    });
}