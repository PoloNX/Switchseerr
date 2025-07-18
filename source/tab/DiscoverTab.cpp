#include "tab/DiscoverTab.hpp"
#include "view/HRecycling.hpp"

DiscoverTab::DiscoverTab(HttpClient& httpClient, AuthService& authService) : httpClient(httpClient), authService(authService) {
    brls::Logger::debug("DiscoverTab: Initializing DiscoverTab");
    this->inflateFromXMLRes("xml/tab/discover.xml"); 
}

void DiscoverTab::willAppear(bool resetState) {
    brls::Logger::debug("DiscoverTab: willAppear called with resetState={}", resetState);
    
    latestMedias = new RecyclingVideo(httpClient, authService);
    latestMedias->recycler->setHeight(400);
    latestMedias->recycler->estimatedRowWidth = 266;
    latestMedias->resume = true;
    latestMedias->pageSize = 6;
    latestMedias->recycler->reloadData();
    latestMedias->setTitle("Trending");

    this->latestMedias->doRequest(true);


    boxLatest->addView(latestMedias);


    
    // Additional setup can be done here if needed
}

// brls::View* DiscoverTab::create() {
//     return new DiscoverTab();
// }