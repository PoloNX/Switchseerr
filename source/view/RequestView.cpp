#include "view/RequestView.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/ThreadPool.hpp"
#include "api/RequestService.hpp"

using namespace brls::literals;

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
        request.is4k = selectedRadarrService.is4k;
        request.serverId = selectedRadarrService.id;
        request.rootFolder = selectedRadarrService.activeDirectory;
        request.profilerId = selectedQualityProfile.id;
        request.languageProfileId = 0; // Assuming language profile ID is not used for movies
        request.userId = authService->getUserId();

        if (requestService.createRequest(request, mediaItem.type)) {
            brls::Logger::debug("RequestView: Request created successfully for media item ID: {}", mediaItem.id);
            brls::Application::notify("main/view/request/success"_i18n);
            return false;
        } else {
            brls::Logger::error("RequestView: Failed to create request for media item ID: {}", mediaItem.id);
            brls::Application::notify("main/view/request/error"_i18n);
            return false;
        }
        return true;
    });
}

void RequestView::loadQualityProfiles() {
    this->selectedQualityProfile = this->selectedRadarrService.qualityProfiles[0];

    std::vector<std::string> profileNames;
    for (const auto& profile : selectedRadarrService.qualityProfiles) {
        profileNames.push_back(profile.name);
    }

    this->qualityCell->setFocusable(true);
    this->qualityCell->setText(profileNames.empty() ? "main/view/request/no_profiles"_i18n : profileNames[0]);
    
    if (this->selectedRadarrService.qualityProfiles.empty()) return;
    
    std::vector<std::string> names;
    for (const auto& profile : this->selectedRadarrService.qualityProfiles) {
        names.push_back(profile.name);
    }

    this->qualityCell->registerClickAction([this, names = std::move(names)](brls::View* view) {        
        int selectedProfileIndex = std::distance(names.begin(), std::find(names.begin(), names.end(), this->selectedQualityProfile.name));

        auto dropdown = new brls::Dropdown("main/view/request/select_profile"_i18n, names, [this](int _selected) {
            if (_selected >= 0 && _selected < this->selectedRadarrService.qualityProfiles.size()) {
                this->selectedQualityProfile = this->selectedRadarrService.qualityProfiles[_selected];
                this->qualityCell->setText(this->selectedQualityProfile.name);
            }
        }, selectedProfileIndex);
        
        brls::Application::pushActivity(new brls::Activity(dropdown));
        return true;
    });
}

void RequestView::loadRadarrProfiles() {
    // Show a dropdown to select the radarr service
    std::vector<std::string> serviceNames;
    for (const auto& service : availableServers) {
        serviceNames.push_back(service.name);
    }
    this->serverCell->setFocusable(true);
    this->serverCell->setText(serviceNames.empty() ? "main/view/request/no_radarr_service"_i18n : serviceNames[0]);

    if (serviceNames.empty()) return;

    this->serverCell->registerClickAction([this, serviceNames](brls::View* view) {
        // Show a dropdown to select the Radarr service
        int selectedServiceIndex = std::distance(serviceNames.begin(), std::find(serviceNames.begin(), serviceNames.end(), this->selectedRadarrService.name));
        auto dropdown = new brls::Dropdown("main/view/request/select_radarr_service"_i18n, serviceNames, [this](int _selected) {
            if (_selected >= 0 && _selected < availableServers.size()) {
                this->selectedRadarrService = availableServers[_selected];
                this->serverCell->setText(this->selectedRadarrService.name);
            }
            loadQualityProfiles();
        }, selectedServiceIndex);
        brls::Application::pushActivity(new brls::Activity(dropdown));

        return true;
    });

}

void RequestView::loadProfiles() {
    brls::Logger::debug("RequestView: Loading profiles for media item ID: {}", mediaItem.id);

    this->qualityCell->setFocusable(false);
    this->qualityCell->setText("main/view/request/loading_profiles"_i18n);

    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        brls::Logger::debug("RequestView: Fetching quality profiles for media item ID: {}", this->mediaItem.id);
        availableServers = jellyseerr::getRadarrServices(client, this->authService->getServerUrl());
        
        if (availableServers.empty()) {
            brls::Logger::warning("RequestView: No Radarr services found");
            brls::sync([ASYNC_TOKEN] {
                ASYNC_RELEASE
                this->serverCell->setVisibility(brls::Visibility::GONE);
                this->serverHeader->setVisibility(brls::Visibility::GONE);
                this->qualityHeader->setVisibility(brls::Visibility::GONE);
                this->qualityCell->setVisibility(brls::Visibility::GONE);
                this->setHeight(250);
            });
        } 
        
        else {
            this->selectedQualityProfile = availableServers[0].qualityProfiles[0]; // Default to the first profile
            this->selectedRadarrService = availableServers[0];  
            brls::sync([ASYNC_TOKEN] {
                ASYNC_RELEASE
                loadRadarrProfiles();
                loadQualityProfiles();
                this->content->setHeight(450);
                this->requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
                loadButtonActions();
            });
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