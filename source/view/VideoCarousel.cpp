#include "view/VideoCarousel.hpp"
#include "view/VideoCard.hpp"
#include "api/Jellyseerr.hpp"
#include "view/MediaPreview.hpp"

static std::mutex downloadMutex;

VideoCarousel::VideoCarousel(HttpClient& httpClient, AuthService& authService, DiscoverType type)
    : httpClient(httpClient), authService(authService), type(type) {
    brls::Logger::debug("VideoCarousel: Creating carousel for type {}", static_cast<int>(type));
    this->inflateFromXMLRes("xml/view/video_carousel.xml");
    this->scrollingFrame->setFocusable(true);
    this->setMargins(10, 0, 10, 0);
    doRequest();
}

VideoCarousel::~VideoCarousel() {
    brls::Logger::debug("VideoCarousel: Destroying carousel for type {}", static_cast<int>(type));
}

void VideoCarousel::doRequest() {
    brls::Logger::debug("VideoCarousel: Requesting data for type {}", static_cast<int>(type));

    std::string serverUrl = authService.getServerUrl();
    std::string apiKey = authService.getToken().value_or("");

    brls::async([this, serverUrl = std::move(serverUrl), apiKey = std::move(apiKey)]() {
        switch(type) {
            case DiscoverType::RecentlyAdded:
                this->header->setTitle("Recently Added");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::RecentlyAdded, 20);
                break;
            case DiscoverType::Trending:
                header->setTitle("Trending");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::Trending, 20);
                break;
            case DiscoverType::PopularMovies:
                header->setTitle("Popular Movies");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::PopularMovies, 20);
                break;
            case DiscoverType::PopularTvShows:
                header->setTitle("Popular TV Shows");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::PopularTvShows, 20);
                break;
            case DiscoverType::FutureMovies:
                header->setTitle("Future Movies");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::FutureMovies, 20);
                break;
            case DiscoverType::FutureTvShows:
                header->setTitle("Future TV Shows");
                items = jellyseerr::getMedias(httpClient, serverUrl, apiKey, DiscoverType::FutureTvShows, 20);
                break;
        }

        for (auto& item : items) {
            brls::sync([this, item]() {
                auto videoCard = new VideoCardCell();
                videoCard->labelTitle->setText(item.title);
                videoCard->labelExt->setText(item.releaseDate.empty() ? item.firstAirDate : item.releaseDate);
                videoCard->labelRating->setText(std::to_string(item.voteAverage));
                videoCard->rectType->setCornerRadius(10); // Make it a pill shape
                videoCard->setBorderThickness(3);
                if (item.type == MediaType::Tv) {
                    videoCard->labelType->setText("Série");
                    videoCard->rectType->setColor(nvgRGBA(147, 51, 234, 0.8*255)); // Example color, adjust as needed
                    videoCard->rectType->setBorderColor(nvgRGBA(147, 51, 234, 255)); // Example border color, adjust as needed
                } else {
                    videoCard->labelType->setText("Film");
                    videoCard->rectType->setColor(nvgRGBA(37, 99, 235, 0.8*255)); // Example color, adjust as needed
                    videoCard->rectType->setBorderColor(nvgRGBA(59, 130, 246, 255)); // Example border color, adjust as needed
                }

                videoCard->setVisibility(brls::Visibility::VISIBLE);

                if (!item.posterPath.empty()) {
                    brls::async([this, item, videoCard]() {
                        brls::Logger::debug("VideoCarousel, Downloading image for item ID: {}", item.title);
                        
                        std::vector<unsigned char> imageBuffer = this->httpClient.downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", item.posterPath));

                        if(!imageBuffer.empty()) {
                            brls::sync([videoCard, item, imageBuffer = std::move(imageBuffer)] {
                                brls::Logger::debug("VideoCarousel, Image downloaded successfully for item ID: {}", item.id);
                                videoCard->picture->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                            });
                        } else {
                            brls::Logger::error("VideoCarousel, Failed to download image for item ID: {}", item.id);
                        }
                    });
                }
                videoCard->setMargins(0, 10, 0, 10);

                videoCard->registerClickAction([this, item](brls::View* view) mutable {
                    brls::Logger::debug("VideoCarousel: Item clicked with ID: {}", item.id);
                    auto mediaPreview = new MediaPreview(httpClient, authService, item);
                    getAppletFrame()->present(mediaPreview);
                    getAppletFrame()->setHeaderVisibility(brls::Visibility::GONE);
                    brls::Application::giveFocus(mediaPreview);
                    return true;
                });

                this->carouselBox->addView(videoCard);
            });
            
        }

    });
    
}