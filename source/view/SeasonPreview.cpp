#include "view/SeasonPreview.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/ThreadPool.hpp"

class EpisodeBox : public brls::Box {
public:
    EpisodeBox(const Episode& episode) : episode(episode) {
        this->inflateFromXMLRes("xml/view/episode_box.xml");
        this->episodeName->setText(episode.name);
        this->episodeOverview->setText(episode.overview.empty() ? "No overview available" : episode.overview);
        this->episodeImage->setImageFromRes("img/place_holder.png");

        loadImage();
    }
private:
    const Episode episode;

    void loadImage() {
        ThreadPool& threadPool = ThreadPool::instance();
        ASYNC_RETAIN
        threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
            std::string imageUrl = fmt::format("https://image.tmdb.org/t/p/original{}", episode.stillPath);
            auto imageBuffer = client->downloadImageToBuffer(imageUrl);
            brls::sync([ASYNC_TOKEN, imageBuffer] {
                ASYNC_RELEASE
                if (!imageBuffer.empty()) {
                    this->episodeImage->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                }
            });
        });
    }

    BRLS_BIND(brls::Label, episodeName, "episode/name");
    BRLS_BIND(brls::Label, episodeOverview, "episode/overview");
    BRLS_BIND(brls::Image, episodeImage, "episode/image");
};

SeasonPreview::SeasonPreview(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, Season& season) : httpClient(httpClient), authService(authService), season(season) {
    this->inflateFromXMLRes("xml/view/season_preview.xml");
    this->titleLabel->setText(season.name);

    loadSeasonDetails();
}

void SeasonPreview::loadSeasonDetails() {
    brls::Logger::debug("SeasonPreview: Loading details for season ID: {}", season.id);
    ThreadPool& threadPool = ThreadPool::instance();
    
    std::string cookieFilePath = httpClient->getCookieFilePath();
    
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN, cookieFilePath](std::shared_ptr<HttpClient> client) {
        client->setCookieFile(cookieFilePath);
        jellyseerr::fillSeasonDetails(client, authService->getServerUrl(), season);
        brls::sync([ASYNC_TOKEN] {
            ASYNC_RELEASE
            for (const auto& episode : season.episodes) {
                brls::Logger::debug("SeasonPreview: Episode {} - Name: {}, Air Date: {}", episode.episodeNumber, episode.name, episode.airDate);
                auto episodeBox = new EpisodeBox(episode);
                this->episodesBox->addView(episodeBox);
            }
        });
    });

    this->backButton->registerClickAction([this](brls::View* view) {
        brls::Application::popActivity();
        return true;
    });

    brls::Application::giveFocus(this->frame);
}


void SeasonPreview::show(std::function<void(void)> cb, bool animate, float animationDuration) {
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

void SeasonPreview::offsetTick()
{
    content->setTranslationY(showOffset);
}

void SeasonPreview::hide(std::function<void(void)> cb, bool animated, float animationDuration)
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