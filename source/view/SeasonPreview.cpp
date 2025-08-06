#include "view/SeasonPreview.hpp"

SeasonPreview::SeasonPreview(const Season& season) : season(season) {
    this->inflateFromXMLRes("xml/view/season_preview.xml");
    this->titleLabel->setText(season.name);
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