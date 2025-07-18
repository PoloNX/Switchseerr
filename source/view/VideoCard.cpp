#include "view/VideoCard.hpp"

using namespace brls::literals;

VideoCardCell::VideoCardCell() {
    this->inflateFromXMLRes("xml/view/video_card.xml");
}