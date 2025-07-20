#pragma once

#include "view/RecyclingGrid.hpp"


class BaseCardCell : public RecyclingGridItem {
public:
    ~BaseCardCell() {}

    void prepareForReuse() override {  }

    void cacheForReuse() override { }

    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelExt, "video/card/label/ext");
    BRLS_BIND(brls::Rectangle, rectType, "video/card/type/rectangle");
    BRLS_BIND(brls::Label, labelType, "video/card/type/label");
};

class MediaCardCell : public BaseCardCell {
public:
    MediaCardCell() { this->inflateFromXMLRes("xml/view/video_card.xml"); }

    static MediaCardCell* create() { return new MediaCardCell(); }
};

class VideoCardCell : public BaseCardCell {
public:
    VideoCardCell();

    static VideoCardCell* create() { return new VideoCardCell(); }

    BRLS_BIND(brls::Label, labelRating, "video/card/label/rating");
};