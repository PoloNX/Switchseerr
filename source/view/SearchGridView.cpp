#include "view/SearchGridView.hpp"
#include "utils/ThreadPool.hpp"

SearchGridView::SearchGridView(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
    : httpClient(httpClient), authService(authService) {
    this->inflateFromXMLRes("xml/view/search_grid.xml");
    data = std::make_unique<MediaGridData>(httpClient, authService, MediaType::Movie);
    recycler->setDataSource(data.get());
    recycler->registerCell("Cell", VideoCardCell::create);
    recycler->estimatedRowSpace = 31;

    ThreadPool& threadPool = ThreadPool::instance();
    
    this->inputLabel->setText("Search for Movies or TV Shows");
    inputLabel->setTextColor(nvgRGBA(255, 255, 255, 255));
    searchIcon->setImageFromSVGRes("icon/icon-search-white.svg");

    searchBox->setFocusable(true);
    searchBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->searchBox));
    searchBox->registerClickAction([this](brls::View* view) {
        brls::Logger::debug("SearchGridView: Search box clicked");
        brls::Application::getImeManager()->openForText(
            [this](const std::string& text) {
                this->currentSearchQuery = text;
                this->updateData();
            }, "Enter search query", "", 64, this->currentSearchQuery, 0);
        return true;
    });
    searchBox->setBorderThickness(4);
    searchBox->setBackgroundColor(nvgRGBA(88, 84, 84, 255));
    searchBox->setCornerRadius(10);
    //searchBox->setWireframeEnabled(true);

    previousPageButton->registerClickAction([this](brls::View* view) {
        onPreviousPage();
        return true;
    });
    nextPageButton->registerClickAction([this](brls::View* view) {
        onNextPage();
        return true;
    });
    pageBox->setVisibility(brls::Visibility::GONE);
}

void SearchGridView::updateData() {
    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN](std::shared_ptr<HttpClient> client) {
        if (currentSearchQuery.empty()) {
            brls::Logger::debug("SearchGridView: No search query");
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                brls::Logger::debug("SearchGridView: No search query, showing input label");
                this->inputLabel->setText("Search for Movies or TV Shows");
                this->pageBox->setVisibility(brls::Visibility::GONE);
            });
            brls::Application::notify("Please enter a search query.");
        } else {
            brls::Logger::debug("SearchGridView: Updating data for query: {}", currentSearchQuery);
            data->setSearchQuery(currentSearchQuery);
            data->loadData(currentPage);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                brls::Logger::debug("SearchGridView: Data updated");
                recycler->notifyDataChanged();
                recycler->reloadData();
                this->inputLabel->setText(currentSearchQuery);
                pageBox->setVisibility(brls::Visibility::VISIBLE);
                pageIndicator->setText(fmt::format("Page {}", currentPage));
            });
        }
    });
}

void SearchGridView::onPreviousPage() {
    if (currentPage > 1) {
        currentPage--;
        brls::Logger::debug("SearchGridView: Going to previous page: {}", currentPage);
        updateData();
    } else {
        brls::Logger::warning("SearchGridView: Already on the first page");
    }
}

void SearchGridView::onNextPage() {
    currentPage++;
    brls::Logger::debug("SearchGridView: Going to next page: {}", currentPage);
    updateData();
}   

SearchGridView::~SearchGridView() {
    // Destructor implementation
}