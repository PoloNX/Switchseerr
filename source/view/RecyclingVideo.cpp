#include "view/RecyclingVideo.hpp"
#include "view/HRecycling.hpp"
#include "view/VideoCard.hpp"
#include "api/Jellyseerr.hpp"
#include "view/VideoSource.hpp"

const std::string recylingVideoContentXML = R"xml(
    <brls:Box
        width="auto"
        height="auto"
        axis="column">

        <brls:Header
            height="40"
            id="recycler/title" />

        <HRecyclerFrame
            id="recycler/videos" />

    </brls:Box>
)xml";

brls::View* RecyclingVideo::create() {
    HttpClient httpClient;
    AuthService authService(httpClient, Config::instance().getUrl());
    return new RecyclingVideo(httpClient, authService);
}

RecyclingVideo::RecyclingVideo(HttpClient& httpClient, AuthService& authService): httpClient(httpClient), authService(authService) {
    brls::Logger::debug("RecyclingVideo : Creating RecyclingVideo view");

    this->inflateFromXMLString(recylingVideoContentXML);

    this->registerStringXMLAttribute("title", [this](std::string value) { this->setTitle(value); });

    this->registerFloatXMLAttribute("frameHeight", [this](float value) { this->recycler->setHeight(value); });

    this->registerFloatXMLAttribute("itemWidth", [this](float value) {
        this->recycler->estimatedRowWidth = value;
        this->recycler->reloadData();
    });

    this->registerFloatXMLAttribute("itemSpace", [this](float value) {
        this->recycler->estimatedRowSpace = value;
        this->recycler->reloadData();
    });

    this->registerFloatXMLAttribute("pageSize", [this](float value) { this->pageSize = value; });

    this->registerAutoXMLAttribute(
        "nextPage", [this]() { this->recycler->onNextPage([this]() { this->doRequest(); }); });

    this->registerBoolXMLAttribute("resume", [this](bool value) { this->resume = value; });

    this->recycler->registerCell("Cell", VideoCardCell::create);
}

RecyclingVideo::~RecyclingVideo() {
    brls::Logger::debug("RecyclingVideo", "Destroying RecyclingVideo view");
}

void RecyclingVideo::setTitle(const std::string& text) { this->title->setTitle(text); }

void RecyclingVideo::doRequest(bool refresh) {
    brls::Logger::debug("RecyclingVideo, doRequest called with refresh={}", refresh);

    if (refresh) {
        this->start = 0;
        this->recycler->showSkeleton(this->pageSize);
    }

    auto latestMedias = jellyseerr::getLatestMedias(this->httpClient, "http://jellyseerr.cabanaflix.ovh", "MTc0NjMwMDcwODg1OTBjNGIyYzY4LWU3YWUtNDE3Ny04ZTZkLTg4MDMxZTA2ZWUxNQ==");
    
    //debug
    for (const auto& media : latestMedias) {
        brls::Logger::debug("Media ID: {}, Title: {}", media.id, media.title);
    }

    this->start = this->pageSize;
    if (latestMedias.empty()) {
        this->setVisibility(brls::Visibility::GONE);
        this->recycler->clearData();
    } else {
        this->setVisibility(brls::Visibility::VISIBLE);
        this->recycler->setDataSource(new VideoDataSource(latestMedias, httpClient));
        this->recycler->reloadData();
    }
}

void RecyclingVideo::doLatest(bool refresh) {
    if (refresh) {
        this->start = 0;
        this->recycler->showSkeleton(this->pageSize);
    }
    ASYNC_RETAIN
    brls::async([ASYNC_TOKEN]() {
        auto latestMedias = jellyseerr::getLatestMedias(this->httpClient, this->authService.getServerUrl(), this->authService.getToken().value_or(""));
        this->start = this->pageSize;
        if (latestMedias.empty()) {
            this->setVisibility(brls::Visibility::GONE);
            this->recycler->clearData();
        } else {
            this->setVisibility(brls::Visibility::VISIBLE);
            // Update the recycler with the new data TODO
            this->recycler->reloadData();
        }
    });
    this->queryCallback(this->start, this->pageSize);
}