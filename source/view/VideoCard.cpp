#include "view/VideoCard.hpp"

using namespace brls::literals;

VideoCardCell::VideoCardCell() {
    this->setBackgroundColor(nvgRGBA(88, 84, 84, 255));
    this->setCornerRadius(10);
    this->inflateFromXMLRes("xml/view/video_card.xml");
}