#include "tab/DiscoverTab.hpp"
#include "view/HRecycling.hpp"
#include "view/VideoCarousel.hpp"

DiscoverTab::DiscoverTab(HttpClient& httpClient, AuthService& authService) : httpClient(httpClient), authService(authService) {
    brls::Logger::debug("DiscoverTab: Initializing DiscoverTab");
    this->inflateFromXMLRes("xml/tab/discover.xml"); 

    
    auto recentlyAdded = new VideoCarousel(httpClient, authService, DiscoverType::RecentlyAdded);
    auto trendingMedias = new VideoCarousel(httpClient, authService, DiscoverType::Trending);
    auto popularMovies = new VideoCarousel(httpClient, authService, DiscoverType::PopularMovies);
    auto popularTvShows = new VideoCarousel(httpClient, authService, DiscoverType::PopularTvShows);
    auto futureMovies = new VideoCarousel(httpClient, authService, DiscoverType::FutureMovies);
    auto futureTvShows = new VideoCarousel(httpClient, authService, DiscoverType::FutureTvShows);
    
    this->boxLatest->addView(recentlyAdded);
    this->boxLatest->addView(trendingMedias);
    this->boxLatest->addView(popularMovies);
    this->boxLatest->addView(popularTvShows);
    this->boxLatest->addView(futureMovies);
    this->boxLatest->addView(futureTvShows);
}

void DiscoverTab::willAppear(bool resetState) {

}

// brls::View* DiscoverTab::create() {
//     return new DiscoverTab();
// }