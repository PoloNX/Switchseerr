#pragma once

#include "view/AutoTabFrame.hpp"

class SettingsTab : public AttachedView {
public:
    SettingsTab();
private:
    BRLS_BIND(brls::RadioCell, btnLogout, "settings/logout");
    BRLS_BIND(brls::SelectorCell, btnLanguage, "settings/language");
    BRLS_BIND(brls::DetailCell, btnAbout, "settings/about");
    BRLS_BIND(brls::SelectorCell, btnTheme, "settings/theme");
};