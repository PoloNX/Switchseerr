#include "tab/MediaTab.hpp"

#include "view/MediaGridView.hpp"

MediaTab::MediaTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, MediaType mediaType)
    : httpClient(httpClient), authService(authService), mediaType(mediaType) {
    this->inflateFromXMLRes("xml/tab/media.xml");
    auto mediaGridView = new MediaGridView(httpClient, authService, mediaType);
    content->addView(mediaGridView);
}

MediaTab::~MediaTab() {
    brls::Logger::debug("MediaTab: destroy");
}