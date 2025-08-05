#include "activity/ServerList.hpp"
#include "tab/ServerAdd.hpp"
#include "tab/ServerLogin.hpp"
#include "view/RecyclingGrid.hpp"
#include "auth/AuthService.hpp"
#include "activity/MainActivity.hpp"

ServerCell::ServerCell(const AppServer& server) {
    this->inflateFromXMLRes("xml/view/server_cell.xml");

    this->setFocusSound(brls::SOUND_FOCUS_SIDEBAR);
    this->registerAction(
        "ok", brls::BUTTON_A, [](brls::View* view) {
            brls::Application::onControllerButtonPressed(brls::BUTTON_NAV_RIGHT, false);
            return true;
        }, false, false, brls::SOUND_CLICK_SIDEBAR
    );

    this->addGestureRecognizer(new brls::TapGestureRecognizer(this));

    this->url->setText(server.url);;
}

void ServerCell::setActive(bool active) {
    auto theme = brls::Application::getTheme();
    if (active) {
        this->accent->setVisibility(brls::Visibility::VISIBLE);
        this->url->setTextColor(theme["brls/sidebar/active_item"]);
    } else {
        this->accent->setVisibility(brls::Visibility::INVISIBLE);
        this->url->setTextColor(theme["brls/text"]);
    }
}

bool ServerCell::getActive() {
    return this->accent->getVisibility() == brls::Visibility::VISIBLE;
}


//RecyclingView implementation

class UserCell : public RecyclingGridItem {
public:
    UserCell() { this->inflateFromXMLRes("xml/view/user_cell.xml");}
    ~UserCell() { }

    BRLS_BIND(brls::Label, name, "user/name");
    BRLS_BIND(brls::Image, picture, "user/avatar");
};

class ServerUserDataSource : public RecyclingGridDataSource {
public:
    ServerUserDataSource(const std::vector<AppUser>& users, ServerList* server) : list(users), parent(server) {}

    size_t getItemCount() override { return this->list.size(); }

    RecyclingGridItem* cellForRow(RecyclingView* recycler, size_t index) override {
        UserCell* cell = dynamic_cast<UserCell*>(recycler->dequeueReusableCell("Cell"));
        auto& u = this->list.at(index);

        cell->registerAction("delete", brls::BUTTON_X, [this, u](brls::View* view) {
            auto dialog = new brls::Dialog("delete");
            dialog->addButton("yes", [this, u]() {
                Config::instance().removeUser(u.id);
                this->parent->onUser(u.server_url);
            });
            return true;
        });

        cell->name->setText(u.name);
        return cell;
    }

    void onItemSelected(brls::Box* recycler, size_t index) override {
        brls::Application::blockInputs();

        brls::async([this, index]() {
            auto& u = this->list.at(index);
            std::shared_ptr<HttpClient> client = this->parent->getHttpClient();
            std::shared_ptr<AuthService> authService = std::make_shared<AuthService>(client, u.server_url);

            if (authService->tryLoginFromCookies(u.name)) {
                brls::sync([this, u, authService = std::move(authService), client]() mutable {
                    brls::Logger::info("ServerUserDataSource: User {} logged in successfully from cookies", u.name);
                    Config::instance().addUser(u, this->parent->getUrl());
                    brls::Application::unblockInputs();
                    brls::Application::clear();
                    brls::Application::pushActivity(new MainActivity(client, authService), brls::TransitionAnimation::NONE);
                });
            }
            else {
                brls::sync([this, u]() {
                    brls::Application::unblockInputs();
                });
                brls::Logger::error("ServerUserDataSource: Failed to log in user {}", u.name);
            }
        });
    }

    void clearData() override { this->list.clear(); }

private:
    std::vector<AppUser> list;
    ServerList* parent;
};


ServerList::ServerList(std::shared_ptr<HttpClient> httpClient) : httpClient(httpClient) {
    brls::Logger::debug("ServerList: create");
}   

void ServerList::onContentAvailable() {
    brls::Logger::debug("ServerList: content available");

    this->btnServerAdd->registerClickAction([this](brls::View* view) {
        view->present(new ServerAdd(httpClient));
        return true;
    });

    this->sidebarServers->registerAction("add", brls::BUTTON_Y, [this](brls::View* view) {
        view->present(new ServerAdd(httpClient));
        return true;
    });

    this->recyclerUsers->registerCell("Cell", [](){ return new UserCell();});

    this->btnSignin->registerClickAction([this](brls::View* view) {
        view->present(new ServerLogin(httpClient, this->getUrl()));
        return true;
    });
}

void ServerList::willAppear(bool resetState) {
    brls::Logger::debug("ServerList: willAppear called");
    auto list = Config::instance().getServers();
    ServerCell* cell = nullptr;

    std::string url = Config::instance().getUrl();
    this->sidebarServers->clearViews();

    for(auto& server : list) {
        cell = new ServerCell(server);
        cell->getFocusEvent()->subscribe([this, server](brls::View* view) {
            this->setActive(view);
            this->onServer(server);
        });

        cell->registerAction("delete", brls::BUTTON_X, [this, server](brls::View* item) {
            auto dialog = new brls::Dialog("delete");
            dialog->addButton("yes", [this, server, item]() {
                if (Config::instance().removeServer(server.url)) {
                    brls::View* view = new ServerAdd(httpClient);
                    this->appletFrame->present(view);
                    brls::Application::giveFocus(view);
                } else {
                    this->sidebarServers->removeView(item);
                }
            });
            return true;
        });

        if (!server.url.empty()) {
            if (url.empty()) {
                url = server.url;
            }
            if (server.url == url) {
                cell->setActive(true);
                this->onServer(server);
            }
        }

        this->sidebarServers->addView(cell);
    }

    if(!cell) {  
        brls::Logger::info("ServerAdd: App startup detected, setting default focus to inputUrl");
        this->appletFrame->setActionAvailable(brls::ControllerButton::BUTTON_B, false);
        this->appletFrame->present(new ServerAdd(httpClient));
    }
}

void ServerList::onServer(const AppServer& s) {
    this->serverVersion->setDetailText(s.version.empty() ? "-" : s.version);
    this->inputUrl->setDetailText(s.url);
    this->onUser(s.url);
}

void ServerList::onUser(const std::string& id) {
    auto users = Config::instance().getUsers(id);
    if (users.empty()) {
        this->recyclerUsers->setEmpty();
        brls::sync([this]() { brls::Application::giveFocus(this->sidebarServers); });
    } else {
        this->recyclerUsers->setDataSource(new ServerUserDataSource(users, this));
    }
}

std::string ServerList::getUrl() { return this->inputUrl->detail->getFullText(); }

void ServerList::setActive(brls::View* active) {
    for (auto item : this->sidebarServers->getChildren()) {
        ServerCell* cell = dynamic_cast<ServerCell*>(item);
        if (cell) cell->setActive(item == active);
    }
}

ServerList::~ServerList() {
    brls::Logger::debug("LoginActivity: destroy");
}