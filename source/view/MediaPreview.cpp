#include "view/MediaPreview.hpp"

MediaPreview::MediaPreview(HttpClient& httpClient, AuthService& authService, MediaItem& mediaItem) : httpClient(httpClient), authService(authService), mediaItem(mediaItem) {
    brls::Logger::debug("MediaPreview: create");
    this->inflateFromXMLRes("xml/view/media_preview.xml");

    this->registerAction("back", brls::BUTTON_B, [this](brls::View* view) {
        brls::Logger::debug("MediaPreview: Back action triggered");
        getAppletFrame()->setHeaderVisibility(brls::Visibility::VISIBLE);
        this->dismiss();
        return true;
    });

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

    if (!mediaItem.posterPath.empty()) {
        brls::async([this, mediaItem]() {
            brls::Logger::debug("VideoCarousel, Downloading image for item ID: {}", mediaItem.title);
            
            std::vector<unsigned char> imageBuffer = this->httpClient.downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", mediaItem.posterPath));

            if(!imageBuffer.empty()) {
                brls::sync([this, mediaItem, imageBuffer = std::move(imageBuffer)] {
                    brls::Logger::debug("VideoCarousel, Image downloaded successfully for item ID: {}", mediaItem.id);
                    this->posterImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                });
            } else {
                brls::Logger::error("VideoCarousel, Failed to download image for item ID: {}", mediaItem.id);
            }
        });
    }

    
    if (!mediaItem.backdropPath.empty()) {
        brls::async([this, mediaItem]() {
            brls::Logger::debug("VideoCarousel, Downloading image for item ID: {}", mediaItem.title);

            std::vector<unsigned char> imageBuffer = this->httpClient.downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1280_and_h720_face{}", mediaItem.backdropPath));

            if(!imageBuffer.empty()) {
                brls::sync([this, mediaItem, imageBuffer = std::move(imageBuffer)] {
                    brls::Logger::debug("VideoCarousel, Image downloaded successfully for item ID: {}", mediaItem.id);
                    if (this->backdropImage) {
                        this->backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                    }
                });
            } else {
                brls::Logger::error("VideoCarousel, Failed to download image for item ID: {}", mediaItem.id);
            }
        });
    }



    // Initialize the MediaPreview components here
    // For example, you might want to set up UI elements to display mediaItem details
}