#include "tab/ServerLogin.hpp"
#include "auth/AuthService.hpp"
#include "activity/MainActivity.hpp"

#include <nlohmann/json.hpp>

ServerLogin::ServerLogin(HttpClient& httpClient, const std::string& serverUrl, const std::string& user) : httpClient(httpClient), url(serverUrl) {
    this->inflateFromXMLRes("xml/tab/server_login.xml");
    brls::Logger::debug("ServerLogin: create");

    this->headerSignin->setTitle(fmt::format("Sign in to {}", serverUrl));
    this->inputUser->init("Username", user);
    this->inputPassword->init("Password", "", [](std::string text){}, "", "", 256);

    this->buttonSignin->setText("Sign in");
    this->buttonSignin->setStyle(&brls::BUTTONSTYLE_PRIMARY);

    this->buttonSignin->registerClickAction([this](...) {
        return this->onSignin();
    });

    this->imageServer->setImageFromRes("img/jellyseerr/logo_stacked.png");
}

ServerLogin::~ServerLogin() {
    brls::Logger::debug("ServerLogin: destroy");
}

bool ServerLogin::onSignin() {
    brls::Logger::debug("ServerLogin: onSignin called");

    std::string user = this->inputUser->getValue();
    std::string password = this->inputPassword->getValue();

    brls::Logger::debug("ServerLogin: user={}, password={}", user, password);

    if (user.empty()) {
        auto dialog = new brls::Dialog("Please enter a username.");
        dialog->addButton("OK", []{});
        dialog->open();
        return false;
    }

    brls::Application::blockInputs();
    this->buttonSignin->setState(brls::ButtonState::DISABLED);

    AuthService authService(this->httpClient, this->url);
    
    if (authService.login(user, password)) {
        brls::Logger::debug("ServerLogin: Login successful for user {}", user);

        // Optionally, you can navigate to another view or perform further actions here
        brls::Application::unblockInputs();
        this->buttonSignin->setState(brls::ButtonState::ENABLED);

        //UserService UserService(this->httpClient, authService);

        brls::Application::clear();
        //brls::Application::pushActivity(new MainActivity(this->httpClient, UserService), brls::TransitionAnimation::FADE);

        return true;
    }
    else {
        auto dialog = new brls::Dialog("Login failed. Please check your credentials.");
        dialog->addButton("OK", []{});
        dialog->open();
        buttonSignin->setState(brls::ButtonState::ENABLED);
        brls::Application::unblockInputs();
        return false;
    }
}
