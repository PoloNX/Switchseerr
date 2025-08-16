#include "view/RequestView.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/ThreadPool.hpp"

using namespace brls::literals;

RequestView::RequestView(std::shared_ptr<HttpClient> httpClient, MediaItem mediaItem, std::shared_ptr<AuthService> authService) : mediaItem(mediaItem), authService(authService), client(httpClient)
{
    this->inflateFromXMLRes("xml/view/request_view.xml");

    mediaLabel->setText(mediaItem.title);

    cancelButton->registerClickAction([this](brls::View *view)
                                      {
        brls::Logger::debug("RequestView: Cancel action triggered");
        brls::Application::popActivity();
        return true; });

    if (mediaItem.type == MediaType::Movie)
    {
        seasonFrame->setVisibility(brls::Visibility::GONE);
    }
    else
    {
        content->setHeight((std::min)(700.0f, (this->authService->getAdvancedRequest() ? 400.0f : 150.0f) + 100.0f * mediaItem.seasons.size() + 50));
        seasonFrame->setHeight((std::min)(250.0f, 100.0f * mediaItem.seasons.size()));
        loadSeasons();
    }

    if(!authService->getAdvancedRequest()) {
        brls::Application::giveFocus(requestButton);
        detailsBox->setVisibility(brls::Visibility::GONE);
    }
    loadProfiles();
    loadImage();
}

void RequestView::loadSeasons()
{
    brls::Logger::debug("RequestView: Loading seasons for media item ID: {}", mediaItem.id);
    if (mediaItem.seasons.empty())
    {
        brls::Logger::warning("RequestView: No seasons available for media item ID: {}", mediaItem.id);
        seasonHeader->setVisibility(brls::Visibility::GONE);
        seasonFrame->setVisibility(brls::Visibility::GONE);
        return;
    }

    for (const auto &season : mediaItem.seasons)
    {
        auto cell = new brls::BooleanCell();
        cell->setWidthPercentage(90.0f);
        brls::Logger::debug("RequestView: Adding season {} with ID {} to the view and with status : {}", season.seasonNumber, season.id, static_cast<int>(season.status));

        if (season.status == MediaStatus::Available || season.status == MediaStatus::PartiallyAvailable || season.status == MediaStatus::Processing || season.status == MediaStatus::Pending)
        {
            cell->setOn(true);
            cell->init(season.id == 0 ? season.name : fmt::format("Season {}", season.seasonNumber), true, [this, season, cell](bool isOn)
                       {
                if (!isOn) {
                    // already selected
                    cell->setOn(true);
                } });
        }
        else
        {
            cell->init(season.id == 0 ? season.name : fmt::format("Season {}", season.seasonNumber), false, [this, season](bool isOn)
                       {
                if (isOn) {
                    selectedSeasons.push_back(season.seasonNumber);
                } else {
                    selectedSeasons.erase(std::remove(selectedSeasons.begin(), selectedSeasons.end(), season.seasonNumber), selectedSeasons.end());
                } });
        }
        if (&season == &mediaItem.seasons.back())
        {
            // Set a custom navigation route for the last season cell
            cell->setCustomNavigationRoute(
                brls::FocusDirection::DOWN,
                serverCell->isFocusable()
                    ? static_cast<brls::View*>(serverCell.getView())
                    : static_cast<brls::View*>(requestButton.getView())
            );
        }

        seasonContent->addView(cell);
    }
}

void RequestView::loadButtonActions()
{
    brls::Logger::debug("RequestView: Loading button actions for media item ID: {}", mediaItem.id);

    requestButton->registerClickAction([this](brls::View *view)
                                       {
        brls::Logger::debug("RequestView: Request action triggered for media item ID: {} and quality profile Name: {}", mediaItem.id, selectedQualityProfile.name);
        if(mediaItem.type == MediaType::Tv) {
            TvRequest request;
            request.mediaId = mediaItem.id;
            request.type = mediaItem.type;
            request.tvdbId = mediaItem.id; // Assuming TVDB ID is the same as media ID for TV shows
            request.selectedSeasons = selectedSeasons;
            request.is4k = selectedService->is4k();
            request.serverId = selectedService->getId();
            request.rootFolder = selectedService->getActiveDirectory();
            request.profileId = selectedQualityProfile.id;
            request.languageProfileId = 0; // Assuming language profile ID is not used for TV shows
            request.userId = authService->getUserId();

            auto sonarrService = std::dynamic_pointer_cast<SonarrService>(selectedService);
            if (sonarrService) {
                if (sonarrService->performRequest(request)) {
                    brls::Logger::debug("RequestView: TV request created successfully for media item ID: {}", mediaItem.id);
                    brls::Application::notify("main/view/request/success"_i18n);
                    brls::Application::popActivity();
                } else {
                    brls::Logger::error("RequestView: Failed to create TV request for media item ID: {}", mediaItem.id);
                }
            } else {
                brls::Logger::error("RequestView: Selected service is not a SonarrService instance for media item ID: {}", mediaItem.id);
            }

        } else {
            MovieRequest request;
            request.mediaId = mediaItem.id;
            request.type = mediaItem.type;
            request.tvdbId = 0; // Assuming TVDB ID is not used for movies
            request.is4k = selectedService->is4k();
            request.serverId = selectedService->getId();
            request.rootFolder = selectedService->getActiveDirectory();
            request.profileId = selectedQualityProfile.id;
            request.languageProfileId = 0; // Assuming language profile ID is not used for movies
            request.userId = authService->getUserId();

            auto radarrService = std::dynamic_pointer_cast<RadarrService>(selectedService);
            if (radarrService) {
                if (radarrService->performRequest(request)) {
                    brls::Logger::debug("RequestView: Movie request created successfully for media item ID: {}", mediaItem.id);
                    brls::Application::notify("main/view/request/success"_i18n);
                    brls::Application::popActivity();
                } else {
                    brls::Logger::error("RequestView: Failed to create movie request for media item ID: {}", mediaItem.id);
                }
            } else {
                brls::Logger::error("RequestView: Selected service is not a RadarrService instance for media item ID: {}", mediaItem.id);
            }

        }

        return true;
    });
}

void RequestView::loadQualityProfiles()
{
    this->selectedQualityProfile = this->selectedService->getQualityProfiles()[0];

    std::vector<std::string> profileNames;
    for (const auto &profile : selectedService->getQualityProfiles())
    {
        profileNames.push_back(profile.name);
    }

    this->qualityCell->setFocusable(true);
    this->qualityCell->setText(profileNames.empty() ? "main/view/request/no_profiles"_i18n : profileNames[0]);

    if (this->selectedService->getQualityProfiles().empty())
        return;

    std::vector<std::string> names;
    for (const auto &profile : this->selectedService->getQualityProfiles())
    {
        names.push_back(profile.name);
    }

    this->qualityCell->registerClickAction([this, names = std::move(names)](brls::View *view) {
        int selectedProfileIndex = std::distance(names.begin(), std::find(names.begin(), names.end(), this->selectedQualityProfile.name));

        auto dropdown = new brls::Dropdown("main/view/request/select_profile"_i18n, names, [this](int _selected) {
            if (_selected >= 0 && _selected < this->selectedService->getQualityProfiles().size()) {
                this->selectedQualityProfile = this->selectedService->getQualityProfiles()[_selected];
                this->qualityCell->setText(this->selectedQualityProfile.name);
            }
        }, selectedProfileIndex);
        
        brls::Application::pushActivity(new brls::Activity(dropdown));
        return true; 
    });
}

void RequestView::loadServerProfiles()
{
    // Show a dropdown to select the radarr service
    std::vector<std::string> serviceNames;
    for (const auto &service : availableServers)
    {
        serviceNames.push_back(service->getServiceName());
    }
    this->serverCell->setFocusable(true);
    this->serverCell->setText(serviceNames.empty() ? "main/view/request/no_radarr_service"_i18n : serviceNames[0]);

    if (serviceNames.empty())
        return;

    this->serverCell->registerClickAction([this, serviceNames](brls::View *view)
                                          {
        // Show a dropdown to select the Radarr service
        int selectedServiceIndex = std::distance(serviceNames.begin(), std::find(serviceNames.begin(), serviceNames.end(), this->selectedService->getServiceName()));
        auto dropdown = new brls::Dropdown("main/view/request/select_radarr_service"_i18n, serviceNames, [this](int _selected) {
            if (_selected >= 0 && _selected < availableServers.size()) {
                this->selectedService = availableServers[_selected];
                this->serverCell->setText(this->selectedService->getServiceName());
            }
            loadQualityProfiles();
        }, selectedServiceIndex);
        brls::Application::pushActivity(new brls::Activity(dropdown));

        return true; });
}

void RequestView::loadProfiles()
{
    brls::Logger::debug("RequestView: Loading profiles for media item ID: {}", mediaItem.id);

    this->qualityCell->setFocusable(false);
    this->qualityCell->setText("main/view/request/loading_profiles"_i18n);

    ThreadPool &threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client)
                      {
        brls::Logger::debug("RequestView: Fetching quality profiles for media item ID: {}", this->mediaItem.id);
        
        if (mediaItem.type == MediaType::Movie) {
            auto radarrServices = jellyseerr::getRadarrServices(client, this->authService, this->authService->getServerUrl());
            for (const auto& service : radarrServices) {
                availableServers.emplace_back(service);
            }
        } else {
            auto sonarrServices = jellyseerr::getSonarrServices(client, this->authService, this->authService->getServerUrl());
            for (const auto& service : sonarrServices) {
                availableServers.emplace_back(service);
            }
        }
        
        if (availableServers.empty()) {
            brls::Logger::warning("RequestView: No Radarr services found");
            brls::sync([ASYNC_TOKEN] {
                ASYNC_RELEASE
                this->serverCell->setVisibility(brls::Visibility::GONE);
                this->serverHeader->setVisibility(brls::Visibility::GONE);
                this->qualityHeader->setVisibility(brls::Visibility::GONE);
                this->qualityCell->setVisibility(brls::Visibility::GONE);
                this->setHeight(this->authService->getAdvancedRequest() ? 250 : 200);
            });
        } 
        
        if(!availableServers.empty()) {
            this->selectedQualityProfile = availableServers[0]->getQualityProfiles()[0]; // Default to the first profile
            this->selectedService = availableServers[0];  
            brls::sync([ASYNC_TOKEN] {
                ASYNC_RELEASE
                loadServerProfiles();
                loadQualityProfiles();
                if(this->mediaItem.type == MediaType::Movie) {
                    this->content->setHeight(this->authService->getAdvancedRequest() ? 450 : 200);
                }
                this->requestButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
                loadButtonActions();
            });
        } });
}

void RequestView::loadImage()
{
    if (mediaItem.posterPath.empty())
    {
        brls::Logger::error("RequestView: Poster path is empty for item ID: {}", mediaItem.id);
        return;
    }

    ThreadPool &threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client)
                      {
        auto imageBuffer = client->downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w1280_and_h720_face{}", this->mediaItem.backdropPath));
        if (!imageBuffer.empty()) {
            brls::sync([ASYNC_TOKEN, imageBuffer = std::move(imageBuffer)] {
                ASYNC_RELEASE
                backdropImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
            });
        } else {
            brls::Logger::error("RequestView: Failed to download backdrop image for item ID: {}", this->mediaItem.id);
        } });
}

void RequestView::show(std::function<void(void)> cb, bool animate, float animationDuration)
{
    if (animate)
    {
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
    if (animated)
    {
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