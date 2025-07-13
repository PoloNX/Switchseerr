#include "tab/ServerAdd.hpp"
#include "api/Jellyseerr.hpp"
#include "utils/Config.hpp"
#include "tab/ServerLogin.hpp"

#include <nlohmann/json.hpp>

ServerAdd::ServerAdd(HttpClient& httpClient) : httpClient(httpClient) {
    this->inflateFromXMLRes("xml/tab/server_add.xml");
    brls::Logger::debug("ServerAdd: create");

    inputUrl->init("URL", "", [](std::string) {}, "http://<Servier IP>:5055", "", 255);

    jellyseerrImage->setImageFromRes("img/jellyseerr/logo_stacked.png");

    connectButton->registerClickAction([this](...) {return this->onConnect();});
    connectButton->setStyle(&brls::BUTTONSTYLE_PRIMARY);
    connectButton->setText("Connect");
}

ServerAdd::~ServerAdd() {
    brls::Logger::debug("ServerAdd: destroy");
}

brls::View* ServerAdd::getDefaultFocus() {
    return this->inputUrl;
}

bool ServerAdd::onConnect() {
    brls::Application::blockInputs();
    connectButton->setState(brls::ButtonState::DISABLED);

    std::string baseUrl = this->inputUrl->getValue();
    if (baseUrl.length() < 10 || baseUrl.substr(0, 4).compare("http")) {
        brls::Application::unblockInputs();
        brls::Logger::error("ServerAdd: Invalid URL: {}", baseUrl);
        connectButton->setState(brls::ButtonState::ENABLED);
        return false;
    }

    while (baseUrl.back() == '/') baseUrl.pop_back(); 
    


    brls::Logger::info("ServerAdd: connecting to server {}", baseUrl);
    std::string response = httpClient.get(fmt::format("{}/api/v1/status", baseUrl));
    brls::Logger::debug("ServerAdd: response: {}", response.c_str());

    nlohmann::json jsonResponse;
    try {
        jsonResponse = nlohmann::json::parse(response);
        jellyseerr::PublicSystemInfo systemInfo = jsonResponse;      
        AppServer server = {
            .name = "Jellyseerr",
            .version = systemInfo.version,
            .url = baseUrl,
        };
        Config::instance().addServer(server);
        brls::Logger::info("ServerAdd: Server added successfully: {}", server.name);
        brls::Application::unblockInputs();

        getAppletFrame()->setActionAvailable(brls::ControllerButton::BUTTON_B, true);
        auto view = new ServerLogin(httpClient, baseUrl);
        this->present(view);
        connectButton->setState(brls::ButtonState::ENABLED);
    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("ServerAdd: JSON parse error: {}", e.what());
        brls::Application::unblockInputs();
        this->connectButton->setState(brls::ButtonState::ENABLED);
        return false;
    }

    return true;
}




