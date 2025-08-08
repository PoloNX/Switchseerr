#include "tab/SettingsTab.hpp"
#include "activity/ServerList.hpp"
#include "utils/Config.hpp"
#include "view/AboutView.hpp"


#include <fmt/ranges.h>
#include <fmt/core.h>

using namespace brls::literals;

SettingsTab::SettingsTab() {
    this->inflateFromXMLRes("xml/tab/settings.xml");

    this->btnLogout->registerClickAction([this](brls::View *view) {
        brls::Logger::debug("SettingsTab: Logout action triggered");
        brls::Application::clear();
        brls::Application::pushActivity(new ServerList());
        return true;
    });

    std::filesystem::path i18n_path = fmt::format("{}i18n", BRLS_RESOURCES);
    std::vector<std::string> languages;
    languages.push_back("auto");
    for (const auto& entry : std::filesystem::directory_iterator(i18n_path)) {
        if (entry.is_directory()) {
            languages.push_back(entry.path().filename().string());
        }
    }

    int selected = 0;
    Config& config = Config::instance();
    std::string currentLanguage = config.getLanguage();
    for(auto i = 0; i < languages.size(); i++) {
        if (languages[i] == currentLanguage) {
            selected = i;
            break;
        }
    }

    brls::Logger::debug("SettingsTab: Available languages: {}", fmt::join(languages, ", "));
    brls::Logger::debug("SettingsTab: Current language: {}", currentLanguage);

    this->btnLanguage->init("main/tab/settings/language"_i18n, languages, selected, [](int selected){}, [languages](int selected) {
        Config& config = Config::instance();
        config.setLanguage(languages[selected]);
    });

    this->btnAbout->registerClickAction([this](brls::View *view) {
        brls::Logger::debug("SettingsTab: About action triggered");
        brls::Application::pushActivity(new brls::Activity( new AboutView()));
        return true;
    });
}