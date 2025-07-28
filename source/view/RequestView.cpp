#include "view/RequestView.hpp"

#include "utils/ThreadPool.hpp"

RequestView::RequestView(MediaItem mediaItem) : mediaItem(mediaItem) {
    this->inflateFromXMLRes("xml/view/request_view.xml");

    mediaLabel->setText(mediaItem.title);
   
    cancelButton->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("RequestView: Cancel action triggered");
        brls::Application::popActivity();

        return true;
    });

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

    brls::Application::notify("Miaou");

}

void RequestView::show(std::function<void(void)> cb, bool animate, float animationDuration) {
    if(animate) {
        content->setTranslationY(30.0f);

        showOffset.stop();
        showOffset.reset(30.0f);
        showOffset.addStep(0, animationDuration, brls::EasingFunction::quadraticOut);
        showOffset.setTickCallback([this]
            { this->offsetTick(); });
        showOffset.start();
    }

    Box::show(cb, animate, animationDuration);

    if (animate) {
        alpha.stop();
        alpha.reset(1);

        applet->alpha.stop();
        applet->alpha.reset(0);
        applet->alpha.addStep(1, animationDuration, brls::EasingFunction::quadraticOut);
        applet->alpha.start();
    }
}

void RequestView::offsetTick()
{
    content->setTranslationY(showOffset);
}

void RequestView::hide(std::function<void(void)> cb, bool animated, float animationDuration)
{

    if (animated)
    {
        alpha.stop();
        alpha.reset(0);

        applet->alpha.stop();
        applet->alpha.reset(1);
        applet->alpha.addStep(0, animationDuration, brls::EasingFunction::quadraticOut);
        applet->alpha.start();
    }

    Box::hide(cb, animated, animationDuration);
}