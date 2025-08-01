#include "tab/SearchTab.hpp"

#include "view/SearchGridView.hpp"

SearchTab::SearchTab(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : httpClient(httpClient), authService(authService), mediaType(mediaType) {
    this->inflateFromXMLRes("xml/tab/search.xml");
    auto searchGridView = new SearchGridView(httpClient, authService);
    content->addView(searchGridView);
}

SearchTab::~SearchTab() {
    brls::Logger::debug("SearchTab: destroy");
}