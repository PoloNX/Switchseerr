#pragma once

#include <borealis.hpp>

enum class LabelBackgroundStyle {
    Movie,
    TVShow,
    Pending,
    Processing,
    PartiallyAvailable,
    Available,
    Blacklisted,
    Deleted
};

class LabelBackground : public brls::Box {
public:
    LabelBackground();

    void setText(const std::string& text);
    void setStyle(LabelBackgroundStyle style);

    static View* create();

private:
    std::string text;
    LabelBackgroundStyle style;

    BRLS_BIND(brls::Label, textLabel, "label");
    BRLS_BIND(brls::Rectangle, backgroundRect, "background");
    BRLS_BIND(brls::Box, labelBox, "box");
};