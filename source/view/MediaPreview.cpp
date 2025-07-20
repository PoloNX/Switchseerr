#include "view/MediaPreview.hpp"

MediaPreview::MediaPreview(HttpClient& httpClient, AuthService& authService, MediaItem& mediaItem) : httpClient(httpClient), authService(authService), mediaItem(mediaItem) {
    brls::Logger::debug("MediaPreview: create");
    this->inflateFromXMLRes("xml/view/media_preview.xml");

    this->registerAction("back", brls::BUTTON_B, [this](brls::View* view) {
        brls::Logger::debug("MediaPreview: Back action triggered");
        getAppletFrame()->setHeaderVisibility(brls::Visibility::VISIBLE);
        this->dismiss();
        return true;
    });

    // Initialize the MediaPreview components here
    // For example, you might want to set up UI elements to display mediaItem details
}