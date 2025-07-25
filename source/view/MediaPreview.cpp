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

void MediaPreview::willAppear(bool resetState) {
    brls::Logger::debug("MediaPreview: willAppear called");

    auto requestButton = new brls::Button();
    requestButton->setText("Request");
    requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
    requestButton->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("MediaPreview: Request button clicked for item ID: {}", mediaItem.id);
        // Handle request logic here
        return true;
    });

    this->actionsBox->addView(requestButton);

}

void MediaPreview::downloadPosterImage() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", mediaItem.posterPath), true);
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                if (this->posterImage) {
                    this->posterImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                } else {
                    brls::Logger::error("MediaPreview, Poster image is null for item ID: {}", mediaItem.id);
                }
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
                if (!this->backdropImage) {
                    brls::Logger::error("MediaPreview: backdropImage is null, cannot set image");
                    return;
                }
                this->backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("MediaPreview, Failed to download backdrop image for item ID: {}", mediaItem.id);
        }
    });
}