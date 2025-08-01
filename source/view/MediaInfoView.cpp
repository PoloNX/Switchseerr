#include "view/MediaInfoView.hpp"

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
        addHeader("Media type", "Movie");
        addHeader("Release Date", mediaItem.releaseDate);
        addHeader("Runtime", mediaItem.runtime > 0 ? std::to_string(mediaItem.runtime) + " minutes" : "");
        addHeader("Original Title", mediaItem.originalTitle);
        addHeader("Revenue", mediaItem.revenue > 0 ? "$" + std::to_string(mediaItem.revenue) : "");
        addHeader("Status", mediaItem.statusString);
    } else {
        addHeader("Media type", "TV Show");
        addHeader("First Air Date", mediaItem.firstAirDate);
        addHeader("In Production", mediaItem.inProduction ? "Yes" : "");
        addHeader("Last Air Date", mediaItem.lasAirDate);
        addHeader("Number of Episodes", mediaItem.numberOfEpisodes > 0 ? std::to_string(mediaItem.numberOfEpisodes) : "");
        addHeader("Number of Seasons", mediaItem.numberOfSeasons > 0 ? std::to_string(mediaItem.numberOfSeasons) : "");
        addHeader("Original Name", mediaItem.originalName);
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