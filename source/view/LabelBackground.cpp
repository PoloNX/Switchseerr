#include "view/LabelBackground.hpp"

LabelBackground::LabelBackground() :text(""), style(LabelBackgroundStyle::Movie) {
    this->inflateFromXMLRes("xml/view/label_background.xml");

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE(
        "style", LabelBackgroundStyle, this->setStyle, {
            {"movie", LabelBackgroundStyle::Movie},
            {"tvshow", LabelBackgroundStyle::TVShow},
            {"pending", LabelBackgroundStyle::Pending},
            {"processing", LabelBackgroundStyle::Processing},
            {"partiallyavailable", LabelBackgroundStyle::PartiallyAvailable},
            {"available", LabelBackgroundStyle::Available},
            {"blacklisted", LabelBackgroundStyle::Blacklisted},
            {"deleted", LabelBackgroundStyle::Deleted}
        });
    
    this->forwardXMLAttribute("text", this->textLabel);

    this->setBorderThickness(3);
    this->backgroundRect->setCornerRadius(10);
}

void LabelBackground::setText(const std::string& text) {
    this->text = text;
    this->textLabel->setText(text);

    this->backgroundRect->setWidth(this->textLabel->getWidth() + 20);
    this->backgroundRect->setHeight(this->textLabel->getHeight() + 12);
}

void LabelBackground::setStyle(LabelBackgroundStyle style) {
    this->style = style;

    switch (style) {
        case LabelBackgroundStyle::Movie:
            this->backgroundRect->setColor(nvgRGBA(37, 99, 235, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(59, 130, 246, 255));
            break;
        case LabelBackgroundStyle::TVShow:
            this->backgroundRect->setColor(nvgRGBA(147, 51, 234, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(147, 51, 234, 255));
            break;
        case LabelBackgroundStyle::Processing:
            this->backgroundRect->setColor(nvgRGBA(99, 102, 241, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(99, 102, 241, 255));
            break;
        case LabelBackgroundStyle::Deleted:
            this->backgroundRect->setColor(nvgRGBA(239, 68, 68, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(239, 68, 68, 255));
            break;
        case LabelBackgroundStyle::PartiallyAvailable :
            this->backgroundRect->setColor(nvgRGBA(34, 197, 94, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(34, 197, 94, 255));
            break;
        case LabelBackgroundStyle::Available:
            this->backgroundRect->setColor(nvgRGBA(34, 197, 94, 0.8 * 255));
            this->backgroundRect->setBorderColor(nvgRGBA(34, 197, 94, 255));
            break;
        default:
            break;
    }

}

brls::View* LabelBackground::create() {
    return new LabelBackground();
}