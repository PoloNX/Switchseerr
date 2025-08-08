#include "view/AboutView.hpp"
#include "utils/Constants.hpp"

AboutView::AboutView()
{
    this->inflateFromXMLRes("xml/view/about_view.xml");

    this->versionLabel->setText(std::string(APP_VERSION));

    this->ghRepoBox->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("AboutView: GitHub repository link clicked");
        brls::Application::getPlatform()->openBrowser("https://github.com/PoloNX/Switchseerr");
        return true;
    });

    brls::Application::giveFocus(this->applet);
}

void AboutView::show(std::function<void(void)> cb, bool animate, float animationDuration)
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

void AboutView::offsetTick()
{
    content->setTranslationY(showOffset);
}

void AboutView::hide(std::function<void(void)> cb, bool animated, float animationDuration)
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