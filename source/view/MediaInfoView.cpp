#include "view/MediaInfoView.hpp"

using namespace brls::literals;

MediaInfoView::MediaInfoView(MediaItem mediaItem) : mediaItem(mediaItem) {
    this->inflateFromXMLRes("xml/view/media_info.xml");
    
    backButton->registerClickAction([this](brls::View* view) {
    brls::Logger::debug("MediaInfoView: Back action triggered");
    brls::Application::popActivity();
    return true;
    });

    setupMediaInfo();
    brls::Application::giveFocus(backButton);
}


void MediaInfoView::setupMediaInfo() {
    if (mediaItem.type == MediaType::Movie) {
        addHeader("main/view/media_info/media_type"_i18n, "main/models/movie"_i18n);
        addHeader("main/view/media_info/release_date"_i18n, mediaItem.releaseDate);
        addHeader("main/view/media_info/runtime"_i18n, mediaItem.runtime > 0 ? std::to_string(mediaItem.runtime) + " minutes" : "");
        addHeader("main/view/media_info/original_title"_i18n, mediaItem.originalTitle);
        addHeader("main/view/media_info/revenue"_i18n, mediaItem.revenue > 0 ? "$" + std::to_string(mediaItem.revenue) : "");
        addHeader("main/view/media_info/status"_i18n, mediaItem.statusString);
    } else {
        addHeader("main/view/media_info/media_type"_i18n, "TV Show");
        addHeader("main/view/media_info/first_air_date"_i18n, mediaItem.firstAirDate);
        addHeader("main/view/media_info/in_production"_i18n, mediaItem.inProduction ? "Yes" : "");
        addHeader("main/view/media_info/last_air_date"_i18n, mediaItem.lasAirDate);
        addHeader("main/view/media_info/number_of_episodes"_i18n, mediaItem.numberOfEpisodes > 0 ? std::to_string(mediaItem.numberOfEpisodes) : "");
        addHeader("main/view/media_info/number_of_seasons"_i18n, mediaItem.numberOfSeasons > 0 ? std::to_string(mediaItem.numberOfSeasons) : "");
        addHeader("main/view/media_info/original_name"_i18n, mediaItem.originalName);
    }
}

void MediaInfoView::addHeader(const std::string& title, const std::string& subtitle) {
    if (subtitle.empty()) {
        return; // Skip empty subtitles
    }
    auto headerType = new brls::Header();
    headerType->setTitle(title);
    headerType->setSubtitle(subtitle);
    detailsBox->addView(headerType);
}

void MediaInfoView::show(std::function<void(void)> cb, bool animate, float animationDuration) {
    if(animate) {
        contentBox->setTranslationY(100.0f);

        showOffset.stop();
        showOffset.reset(100.0f);
        showOffset.addStep(0, animationDuration, brls::EasingFunction::quinticOut);
        showOffset.setTickCallback([this]
            { this->offsetTick(); });
        showOffset.start();
    }

    Box::show(cb, animate, animationDuration);
}

void MediaInfoView::offsetTick()
{
    contentBox->setTranslationY(showOffset);
}

void MediaInfoView::hide(std::function<void(void)> cb, bool animated, float animationDuration)
{
    if(animated) {
        contentBox->setTranslationY(0.0f);

        showOffset.stop();
        showOffset.reset(0.0f);
        showOffset.addStep(100.0f, animationDuration, brls::EasingFunction::quinticOut);
        showOffset.setTickCallback([this]
            { this->offsetTick(); });
        showOffset.start();
    }

    Box::hide(cb, animated, animationDuration);
}