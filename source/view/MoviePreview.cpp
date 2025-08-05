#include "view/MoviePreview.hpp"
#include "utils/ThreadPool.hpp"
#include "view/RequestView.hpp"
#include "view/MediaInfoView.hpp"
#include "api/Jellyseerr.hpp"

MoviePreview::MoviePreview(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaItem& mediaItem, brls::View* parentView) : httpClient(httpClient), authService(authService), mediaItem(mediaItem) {
    brls::Logger::debug("MoviePreview: create for item ID: {}", mediaItem.id);
    this->inflateFromXMLRes("xml/view/movie_preview.xml");

    this->registerAction("back", brls::BUTTON_B, [this, parentView](brls::View* view) {
        brls::Logger::debug("MoviePreview: Back action triggered");
        this->dismiss();
        brls::Application::giveFocus(parentView);
        return true;
    });

    this->scroller->setFocusable(true);

    this->titleLabel->setText(mediaItem.title);
    this->overviewLabel->setText(mediaItem.overview);

    brls::Logger::debug("MoviePreview: Media item type: {}", static_cast<int>(mediaItem.status));

    switch(mediaItem.status) {
        case MediaStatus::Pending:
            this->statusLabel->setStyle(LabelBackgroundStyle::Pending);
            this->statusLabel->setText("Pending");
            break;
        case MediaStatus::Processing:
            this->statusLabel->setStyle(LabelBackgroundStyle::Processing);  
            this->statusLabel->setText("Processing");
            break;
        case MediaStatus::PartiallyAvailable:
            this->statusLabel->setStyle(LabelBackgroundStyle::PartiallyAvailable);
            this->statusLabel->setText("Partially Available");
            break;
        case MediaStatus::Available:
            this->statusLabel->setStyle(LabelBackgroundStyle::Available);
            this->statusLabel->setText("Available");
            break;
        case MediaStatus::Blacklisted:
            this->statusLabel->setStyle(LabelBackgroundStyle::Blacklisted);
            this->statusLabel->setText("Blacklisted");
            break;
        case MediaStatus::Deleted:
            this->statusLabel->setStyle(LabelBackgroundStyle::Deleted);
            this->statusLabel->setText("Deleted");
            break;
        default:
            this->statusLabel->setVisibility(brls::Visibility::GONE);
    }
 
    brls::Application::giveFocus(scroller);
    
    this->actionsBox->setVisibility(brls::Visibility::GONE);

    downloadDetails();
    downloadPosterImage();
    downloadBackdropImage();
}

void MoviePreview::willAppear(bool resetState) {
    brls::Logger::debug("MoviePreview: willAppear called");

    if(mediaItem.status != MediaStatus::Available) {
        auto requestButton = new brls::Button();
        if (mediaItem.status != MediaStatus::PartiallyAvailable) {
            requestButton->setText("Request");
            requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
        } else {
            requestButton->setText("Request More Content");
            requestButton->setStyle(&brls::BUTTONSTYLE_HIGHLIGHT);
        }
        requestButton->registerClickAction([this](brls::View* view) {
            auto requestView = new RequestView(this->httpClient, this->mediaItem, this->authService);
            brls::Application::pushActivity(new brls::Activity(requestView));
            return true;
        });
        requestButton->setMargins(0, 10, 0, 10);
        this->actionsBox->addView(requestButton);
    }


    auto infoButton = new brls::Button();
    infoButton->setText("Info");
    infoButton->setStyle(&brls::BUTTONSTYLE_HIGHLIGHT);
    infoButton->registerClickAction([this](brls::View* view) {
        auto mediaInfoView = new MediaInfoView(this->mediaItem);
        brls::Application::pushActivity(new brls::Activity(mediaInfoView));
        return true;
    });
    infoButton->setMargins(0, 10, 0, 10);
    this->actionsBox->addView(infoButton);
}

void MoviePreview::downloadPosterImage() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", mediaItem.posterPath));
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                if (this->posterImage) {
                    this->posterImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                } else {
                    brls::Logger::error("MoviePreview, Poster image is null for item ID: {}", mediaItem.id);
                }
            });
        } else {
            brls::Logger::error("MoviePreview, Failed to download poster image for item ID: {}", mediaItem.id);
        }
    });
}

void MoviePreview::downloadDetails() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        // Fetch additional details like genres, release date, etc.
        jellyseerr::fillDetails(client, mediaItem, authService->getServerUrl());
        brls::sync([ASYNC_TOKEN] {
            ASYNC_RELEASE
            // Update the UI with the fetched details
            this->actionsBox->setVisibility(brls::Visibility::VISIBLE);
        });
    });
}

void MoviePreview::downloadBackdropImage() {
    auto& threadPool = ThreadPool::instance();

    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = httpClient->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1280_and_h720_face{}", mediaItem.backdropPath));
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                if (!this->backdropImage) {
                    brls::Logger::error("MoviePreview: backdropImage is null, cannot set image");
                    return;
                }
                this->backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("MoviePreview, Failed to download backdrop image for item ID: {}", mediaItem.id);
        }
    });
}