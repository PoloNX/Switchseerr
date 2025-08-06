#include "tab/ServerLogin.hpp"
#include "auth/AuthService.hpp"
#include "activity/MainActivity.hpp"
#include "utils/ThreadPool.hpp"

#include <nlohmann/json.hpp>

using namespace brls::literals;

ServerLogin::ServerLogin(std::shared_ptr<HttpClient> httpClient, const std::string& serverUrl, const std::string& user) : httpClient(httpClient), url(serverUrl) {
    this->inflateFromXMLRes("xml/tab/server_login.xml");
    brls::Logger::debug("ServerLogin: create");

    this->headerSignin->setTitle(fmt::format("{} {}", "main/tab/server_login/sign_in_with"_i18n, "main/tab/server_login/jellyseerr"_i18n));
    this->inputUser->registerClickAction([this](brls::View* view) {
        brls::Application::getImeManager()->openForText(
        [this](const std::string& text) {
            this->username = text;
            this->usernameLabel->setText(text);
        }, "main/tab/server_login/enter_username"_i18n, "", 64, this->username, 0);

        return true;
    });

    this->inputUser->addGestureRecognizer(new brls::TapGestureRecognizer(this->inputUser));
    this->connectionCell->serverName->setText("Loading...");
    this->inputPassword->registerClickAction([this](brls::View* view) {
        brls::Application::getImeManager()->openForPassword(
        [this](const std::string& text) {
            this->password = text;
            // don't show the password but only dots
            this->passwordLabel->setText(std::string(text.length(), '*'));
        }, "main/tab/server_login/enter_password"_i18n, "", 64, this->password);

        return true;
    });

    this->inputPassword->addGestureRecognizer(new brls::TapGestureRecognizer(this->inputPassword));

    this->buttonSignin->setText("main/tab/server_login/sign_in"_i18n);
    this->buttonSignin->setStyle(&brls::BUTTONSTYLE_PRIMARY);

    this->buttonSignin->registerClickAction([this](...) {
        return this->onSignin();
    });

    this->imageServer->setImageFromRes("img/jellyseerr/logo_stacked.png");

    ThreadPool& threadPool = ThreadPool::instance();
    ASYNC_RETAIN
    threadPool.submit([ASYNC_TOKEN, serverUrl, user](std::shared_ptr<HttpClient> client) {
        brls::Logger::debug("ServerLogin: Fetching server image for {}", serverUrl);
        jellyseerr::PublicServerInfo serverInfo = jellyseerr::getPublicServerInfo(client, serverUrl);
        
        brls::sync([ASYNC_TOKEN, serverInfo]() {
            ASYNC_RELEASE
            this->handleServerInfoResponse(serverInfo);
        });
    });
}

void ServerLogin::handleServerInfoResponse(const jellyseerr::PublicServerInfo& serverInfo) {
    if (serverInfo.mediaServerLogin) {
        this->setupMediaServerLogin(serverInfo.serverType);
        brls::Logger::debug("ServerLogin: Media server login enabled, server type: {}", static_cast<int>(this->serverType));
    } else {
        otherBox->setVisibility(brls::Visibility::GONE);
        brls::Logger::debug("ServerLogin: Media server login not enabled");
    }
}

void ServerLogin::setupMediaServerLogin(jellyseerr::ConnectionServer serverType) {
    this->mediaServerLogin = true;
    this->serverType = serverType;
    
    this->updateUIForServerType();
    this->setupConnectionToggle();
    
    this->usernameLabel->setText("main/tab/server_login/username"_i18n);
    this->passwordLabel->setText("main/tab/server_login/password"_i18n);
}

void ServerLogin::updateUIForServerType() {
    std::string title = (this->mediaServerLogin) ? 
        fmt::format("{} {}", "main/tab/server_login/sign_in_with"_i18n, "main/tab/server_login/jellyfin"_i18n) : fmt::format("{} {}", "main/tab/server_login/sign_in_with"_i18n, "main/tab/server_login/jellyseerr"_i18n);
    
    this->headerSignin->setTitle(title);
    // update de connection cell with the other server type to make a button to toggle
    if (this->mediaServerLogin) {
        connectionCell->setServerType(jellyseerr::ConnectionServer::LOCAL);
    } else {
        connectionCell->setServerType(serverType);
    }
}

void ServerLogin::setupConnectionToggle() {
    connectionCell->setFocusable(true);
    connectionCell->addGestureRecognizer(new brls::TapGestureRecognizer(connectionCell));
    connectionCell->registerClickAction([this](brls::View* view) {
        this->toggleMediaServerConnection();
        return true;
    });
}

void ServerLogin::toggleMediaServerConnection() {
    this->mediaServerLogin = !this->mediaServerLogin;
    
    this->updateUIForServerType();
}

ServerLogin::~ServerLogin() {
    brls::Logger::debug("ServerLogin: destroy");
}

bool ServerLogin::onSignin() {
    brls::Logger::debug("ServerLogin: onSignin called");
    brls::Logger::debug("ServerLogin: username={}, password={}", username, password);

    if (username.empty()) {
        auto dialog = new brls::Dialog("main/tab/server_login/username_required"_i18n);
        dialog->addButton("OK", []{});
        dialog->open();
        return false;
    }

    brls::Application::blockInputs();
    this->buttonSignin->setState(brls::ButtonState::DISABLED);

    std::shared_ptr<AuthService> authService = std::make_shared<AuthService>(this->httpClient, this->url);
    
    bool loginSuccess = false;

    // Déterminer quelle méthode de login utiliser
    if (mediaServerLogin) {
        switch (serverType) {
            case jellyseerr::ConnectionServer::JELLYFIN:
                loginSuccess = authService->loginWithJellyfin(username, password);
                break;
            case jellyseerr::ConnectionServer::PLEX:
                loginSuccess = authService->loginWithPlex();
                break;
            default:
                brls::Logger::error("ServerLogin: Unknown server type");
                break;
        }
    } else {
        loginSuccess = authService->loginWithLocal(username, password);
    }

    // Traiter le résultat du login
    if (loginSuccess) {
        brls::Logger::debug("ServerLogin: Login successful for user {}", username);
        brls::Application::unblockInputs();
        this->buttonSignin->setState(brls::ButtonState::ENABLED);
        
        brls::Application::clear();
        brls::Application::pushActivity(new MainActivity(this->httpClient, authService), brls::TransitionAnimation::NONE);
        return true;
    } else {
        auto dialog = new brls::Dialog("main/tab/server_login/login_failed"_i18n);
        dialog->addButton("hints/ok"_i18n, []{});
        dialog->open();
        
        buttonSignin->setState(brls::ButtonState::ENABLED);
        brls::Application::unblockInputs();
        return false;
    }
}
