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

    Theme currentTheme = config.getTheme();
    std::vector<std::string> themes = {"auto", "dark", "light"};
    int themeSelected = 0;
    // Convert currentTheme to string for comparison
    std::string currentThemeStr;
    switch (currentTheme) {
        case Theme::Auto: currentThemeStr = "auto"; break;
        case Theme::Dark: currentThemeStr = "dark"; break;
        case Theme::Light: currentThemeStr = "light"; break;
        default: currentThemeStr = "auto"; break;
    }
    brls::Logger::debug("SettingsTab: Current theme: {}", currentThemeStr);
    for (size_t i = 0; i < themes.size(); i++) {
        if (themes[i] == currentThemeStr) {
            themeSelected = i;
            break;
        }
    }

    this->btnTheme->init("main/tab/settings/theme"_i18n, themes, themeSelected, [](int themeSelected){}, [themes](int themeSelected) {
        Config& config = Config::instance();
        Theme themeEnum = Theme::Auto;
        if (themes[themeSelected] == "dark")
            themeEnum = Theme::Dark;
        else if (themes[themeSelected] == "light")
            themeEnum = Theme::Light;
        else
            themeEnum = Theme::Auto;
        config.setTheme(themeEnum);
    });

    this->btnAbout->registerClickAction([this](brls::View *view) {
        brls::Logger::debug("SettingsTab: About action triggered");
        brls::Application::pushActivity(new brls::Activity( new AboutView()));
        return true;
    });
}