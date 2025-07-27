#include "tab/DiscoverTab.hpp"
#include "view/HRecycling.hpp"
#include "view/VideoCarousel.hpp"
#include "utils/ThreadPool.hpp"

DiscoverTab::DiscoverTab(std::shared_ptr<HttpClient> httpClient, AuthService& authService) : httpClient(httpClient), authService(authService) {
    brls::Logger::debug("DiscoverTab: Initializing DiscoverTab");
    this->inflateFromXMLRes("xml/tab/discover.xml"); 

    // Définir les types de carousels à créer
    std::vector<DiscoverType> carouselTypes = {
        DiscoverType::RecentlyAdded,
        DiscoverType::Trending,
        DiscoverType::PopularMovies,
        DiscoverType::PopularTvShows,
        DiscoverType::FutureMovies,
        DiscoverType::FutureTvShows
    };

    // Créer tous les carousels sans faire leurs requêtes
    std::vector<VideoCarousel*> carousels;
    for (const auto& type : carouselTypes) {
        auto carousel = new VideoCarousel(httpClient, authService, type);
        carousels.push_back(carousel);
        this->boxLatest->addView(carousel);
    }
    
    for (auto* carousel : carousels) {
        carousel->doRequest();
    }

    this->registerAction("Reload", brls::BUTTON_Y, [this, carousels](brls::View* view) {
        brls::Logger::debug("VideoCarousel: Reload action triggered");
        for (auto* carousel : carousels) {
            carousel->doRequest();
        }
        return true;
    });
}

DiscoverTab::~DiscoverTab() {
    brls::Logger::debug("DiscoverTab: Destroying DiscoverTab");
}
