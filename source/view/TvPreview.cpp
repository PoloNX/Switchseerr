#include "view/TvPreview.hpp"
#include "utils/ThreadPool.hpp"
#include "view/RequestView.hpp"
#include "view/MediaInfoView.hpp"
#include "api/Jellyseerr.hpp"
#include "view/SeasonPreview.hpp"

using namespace brls::literals;

class SeasonBox : public brls::Box {
    public:
        SeasonBox(std::shared_ptr<HttpClient>httpClient, std::shared_ptr<AuthService> authService, Season& season) : httpClient(httpClient), authService(authService), season(season) {
            this->inflateFromXMLRes("xml/view/season_box.xml");
            this->seasonName->setText(season.name);
            this->episodeCount->setText(fmt::format("{} {}", season.episodeCount, "main/models/episodes"_i18n));
            this->airDate->setText(season.airDate.empty() ? "main/view/models/unknown_air_date"_i18n : season.airDate);
            this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
            this->registerClickAction([this](brls::View* view) mutable {
                brls::Logger::debug("SeasonBox: Season {} clicked", this->season.seasonNumber);
                brls::Application::pushActivity(new brls::Activity(new SeasonPreview(this->httpClient, this->authService, this->season)));
                return true;
            });
        }
    private:
        std::shared_ptr<HttpClient> httpClient;
        std::shared_ptr<AuthService> authService;
        Season season;

        BRLS_BIND(brls::Label, seasonName, "season/title");
        BRLS_BIND(brls::Label, episodeCount, "season/episodeCount");
        BRLS_BIND(brls::Label, airDate, "season/airDate");
};


TvPreview::TvPreview(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaItem& mediaItem, brls::View* parentView) : httpClient(httpClient), authService(authService), mediaItem(mediaItem) {
    brls::Logger::debug("TvPreview: create for item ID: {}", mediaItem.id);
    this->inflateFromXMLRes("xml/view/tv_preview.xml");

    this->registerAction("back", brls::BUTTON_B, [this, parentView](brls::View* view) {
        brls::Logger::debug("TvPreview: Back action triggered");
        this->dismiss();
        brls::Application::giveFocus(parentView);
        return true;
    });

    this->scroller->setFocusable(true);

    this->titleLabel->setText(mediaItem.title);
    this->overviewLabel->setText(mediaItem.overview);

    brls::Logger::debug("TvPreview: Media item type: {}", static_cast<int>(mediaItem.status));

    switch(mediaItem.status) {
        case MediaStatus::Pending:
            this->statusLabel->setStyle(LabelBackgroundStyle::Pending);
            this->statusLabel->setText("main/view/media_preview/pending"_i18n);
            break;
        case MediaStatus::Processing:
            this->statusLabel->setStyle(LabelBackgroundStyle::Processing);
            this->statusLabel->setText("main/view/media_preview/processing"_i18n);
            break;
        case MediaStatus::PartiallyAvailable:
            this->statusLabel->setStyle(LabelBackgroundStyle::PartiallyAvailable);
            this->statusLabel->setText("main/view/media_preview/partially_available"_i18n);
            break;
        case MediaStatus::Available:
            this->statusLabel->setStyle(LabelBackgroundStyle::Available);
            this->statusLabel->setText("main/view/media_preview/available"_i18n);
            break;
        case MediaStatus::Blacklisted:
            this->statusLabel->setStyle(LabelBackgroundStyle::Blacklisted);
            this->statusLabel->setText("main/view/media_preview/blacklisted"_i18n);
            break;
        case MediaStatus::Deleted:
            this->statusLabel->setStyle(LabelBackgroundStyle::Deleted);
            this->statusLabel->setText("main/view/media_preview/deleted"_i18n);
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

void TvPreview::willAppear(bool resetState) {
    brls::Logger::debug("TvPreview: willAppear called");

    if(mediaItem.status != MediaStatus::Available) {
        auto requestButton = new brls::Button();
        if (mediaItem.status != MediaStatus::PartiallyAvailable) {
            requestButton->setText("main/view/media_preview/request"_i18n);
            requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
        } else {
            requestButton->setText("main/view/media_preview/request_more_content"_i18n);
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
    infoButton->setText("main/view/media_preview/info"_i18n);
    infoButton->setStyle(&brls::BUTTONSTYLE_HIGHLIGHT);
    infoButton->registerClickAction([this](brls::View* view) {
        auto mediaInfoView = new MediaInfoView(this->mediaItem);
        brls::Application::pushActivity(new brls::Activity(mediaInfoView));
        return true;
    });
    infoButton->setMargins(0, 10, 0, 10);
    this->actionsBox->addView(infoButton);
}

void TvPreview::loadSeasons() {
    brls::Logger::debug("TvPreview: loadSeasons called for item ID: {}", mediaItem.id);
    if (mediaItem.numberOfSeasons == 0) {
        brls::Logger::debug("TvPreview: No seasons available for item ID: {}", mediaItem.id);
        return;
    }

    //debug: print all seasons
    for (const auto& season : mediaItem.seasons) {
        brls::Logger::debug("TvPreview: Season {} - Name: {}, Episodes: {}, Air Date: {}", season.seasonNumber, season.name, season.episodeCount, season.airDate);
    }

    for (auto& season : mediaItem.seasons) {
        auto seasonBox = new SeasonBox(httpClient, authService, season);
        seasonBox->setMargins(10, 0, 10, 0);
        seasonsBox->addView(seasonBox);
    }
}

void TvPreview::downloadPosterImage() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w780{}", mediaItem.posterPath));
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                if (this->posterImage) {
                    this->posterImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                } else {
                    brls::Logger::error("TvPreview, Poster image is null for item ID: {}", mediaItem.id);
                }
            });
        } else {
            brls::Logger::error("TvPreview, Failed to download poster image for item ID: {}", mediaItem.id);
        }
    });
}

void TvPreview::downloadDetails() {
    auto& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        // Fetch additional details like genres, release date, etc.
        jellyseerr::fillDetails(client, mediaItem, authService->getServerUrl());
        brls::sync([ASYNC_TOKEN] {
            ASYNC_RELEASE
            // Update the UI with the fetched details
            loadSeasons();
            this->actionsBox->setVisibility(brls::Visibility::VISIBLE);
        });
    });
}

void TvPreview::downloadBackdropImage() {
    auto& threadPool = ThreadPool::instance();

    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        std::vector<unsigned char> imageBuffer = httpClient->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1920_and_h1080_face{}", mediaItem.backdropPath));
        if(!imageBuffer.empty()) { 
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                if (!this->backdropImage) {
                    brls::Logger::error("TvPreview: backdropImage is null, cannot set image");
                    return;
                }
                this->backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("TvPreview, Failed to download backdrop image for item ID: {}", mediaItem.id);
        }
    });
}