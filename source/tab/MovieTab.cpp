#include "tab/MovieTab.hpp"

#include "view/MediaGridView.hpp"

MovieTab::MovieTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : httpClient(httpClient), authService(authService) {
    this->inflateFromXMLRes("xml/tab/movie.xml");
    auto mediaGridView = new MediaGridView(httpClient, authService, MediaType::Movie);
    content->addView(mediaGridView);
}

MovieTab::~MovieTab() {
    brls::Logger::debug("MovieTab: destroy");
}