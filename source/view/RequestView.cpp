#include "view/RequestView.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/ThreadPool.hpp"
#include "api/RequestService.hpp"

RequestView::RequestView(std::shared_ptr<HttpClient> httpClient, MediaItem mediaItem, std::shared_ptr<AuthService> authService) : mediaItem(mediaItem), authService(authService), client(httpClient) {
    this->inflateFromXMLRes("xml/view/request_view.xml");

    mediaLabel->setText(mediaItem.title);
   
    cancelButton->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("RequestView: Cancel action triggered");
        brls::Application::popActivity();
        return true;
    });

    loadProfiles();
    loadImage();
}

void RequestView::loadButtonActions() {
    brls::Logger::debug("RequestView: Loading button actions for media item ID: {}", mediaItem.id);

    requestButton->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("RequestView: Request action triggered for media item ID: {} and quality profile Name: {}", mediaItem.id, selectedQualityProfile.name);
        RequestService requestService(client, authService);
        MovieRequest request;
        request.mediaId = mediaItem.id;
        request.type = mediaItem.type;
        request.tvdbId = 0; // Assuming TVDB ID is not used for movies
        request.seasons = {}; // No seasons for movies
        request.is4k = selectedQualityProfile.radarrService.is4k;
        request.serverId = selectedQualityProfile.radarrService.id;
        request.profilerId = selectedQualityProfile.id;
        request.rootFolder = selectedQualityProfile.radarrService.activeDirectory;
        request.languageProfileId = 0; // Assuming language profile ID is not used for movies
        request.userId = authService->getUserId();

        if (requestService.createRequest(request, mediaItem.type)) {
            brls::Logger::debug("RequestView: Request created successfully for media item ID: {}", mediaItem.id);
            brls::Application::notify("Request created successfully!");
            return false;
        } else {
            brls::Logger::error("RequestView: Failed to create request for media item ID: {}", mediaItem.id);
            brls::Application::notify("Failed to create request for media item. Please try again later.");
            return false;
        }
        return true;
    });
}

void RequestView::loadProfiles() {
    brls::Logger::debug("RequestView: Loading profiles for media item ID: {}", mediaItem.id);

    this->qualityCell->setFocusable(false);
    this->qualityCell->setText("Loading profiles...");

    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        brls::Logger::debug("RequestView: Fetching quality profiles for media item ID: {}", this->mediaItem.id);
        auto radarrServices = jellyseerr::getRadarrServices(client, this->authService->getServerUrl(), this->authService->getToken().value_or(""));
        if (radarrServices.empty()) {
            brls::Logger::warning("RequestView: No Radarr services found");
            brls::sync([ASYNC_TOKEN] {
                ASYNC_RELEASE
                qualityCell->setVisibility(brls::Visibility::GONE);
            });
        } 
        
        else {
            auto qualityProfiles = jellyseerr::getRadarrQualityProfiles(client, authService->getServerUrl(), authService->getToken().value_or(""), radarrServices[0].id);
            if (qualityProfiles.empty()) {
                brls::Logger::warning("RequestView: No quality profiles found for Radarr service ID: {}", radarrServices[0].id);
                brls::sync([ASYNC_TOKEN] {
                    ASYNC_RELEASE
                    qualityCell->setVisibility(brls::Visibility::GONE);
                });
            } else {
                // Generate a vector<string> for profile names
                this->selectedQualityProfile = qualityProfiles[0]; // Default to the first profile

                std::vector<std::string> profileNames;
                for (const auto& profile : qualityProfiles) {
                    profileNames.push_back(profile.name);
                }

                brls::sync([ASYNC_TOKEN, profileNames, qualityProfiles] {
                    ASYNC_RELEASE
                    this->qualityCell->setFocusable(true);
                    this->qualityCell->setText(profileNames.empty() ? "No profiles" : profileNames[0]);
                    
                    this->requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
                    loadButtonActions();

                    this->availableQualityProfiles = qualityProfiles;
                    
                    this->qualityCell->registerClickAction([this](brls::View* view) {
                        if (this->availableQualityProfiles.empty()) return true;
                        
                        std::vector<std::string> names;
                        for (const auto& profile : this->availableQualityProfiles) {
                            names.push_back(profile.name);
                        }
                        
                        int selectedProfileIndex = std::distance(names.begin(), std::find(names.begin(), names.end(), this->selectedQualityProfile.name));

                        auto dropdown = new brls::Dropdown("Select Quality Profile", names, [this](int _selected) {
                            if (_selected >= 0 && _selected < this->availableQualityProfiles.size()) {
                                this->selectedQualityProfile = this->availableQualityProfiles[_selected];
                                this->qualityCell->setText(this->selectedQualityProfile.name);
                            }
                        }, selectedProfileIndex);
                        
                        brls::Application::pushActivity(new brls::Activity(dropdown));
                        return true;
                    });
                });
            }
        }
    });
}

void RequestView::loadImage() {
    if (mediaItem.posterPath.empty()) {
        brls::Logger::error("RequestView: Poster path is empty for item ID: {}", mediaItem.id);
        return;
    }

    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        auto imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1280_and_h720_face{}", this->mediaItem.backdropPath));
        if (!imageBuffer.empty()) {
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("RequestView: Failed to download backdrop image for item ID: {}", this->mediaItem.id);
        }
    }); 
}

void RequestView::show(std::function<void(void)> cb, bool animate, float animationDuration) {
    if(animate) {
        content->setTranslationY(100.0f);

        showOffset.stop();
        showOffset.reset(100.0f);
        showOffset.addStep(0, animationDuration, brls::EasingFunction::quinticOut);
        showOffset.setTickCallback([this]
            { this->offsetTick(); });
        showOffset.start();
    }

    Box::show(cb, animate, animationDuration);
}

void RequestView::offsetTick()
{
    content->setTranslationY(showOffset);
}

void RequestView::hide(std::function<void(void)> cb, bool animated, float animationDuration)
{
    if(animated) {
        content->setTranslationY(0.0f);

        showOffset.stop();
        showOffset.reset(0.0f);
        showOffset.addStep(100.0f, animationDuration, brls::EasingFunction::quinticOut);
        showOffset.setTickCallback([this]
            { this->offsetTick(); });
        showOffset.start();
    }

    Box::hide(cb, animated, animationDuration);
}