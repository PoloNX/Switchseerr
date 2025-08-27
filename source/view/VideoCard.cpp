#include "view/VideoCard.hpp"

using namespace brls::literals;

VideoCardCell::VideoCardCell() {
    this->setCornerRadius(10);
    this->inflateFromXMLRes("xml/view/video_card.xml");
}