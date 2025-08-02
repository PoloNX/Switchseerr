#pragma once

#include "view/RecyclingGrid.hpp"
#include "view/LabelBackground.hpp"
#include "view/SvgImage.hpp"
#include "models/MediaItem.hpp"

class BaseCardCell : public RecyclingGridItem {
public:
    ~BaseCardCell() {}

    void prepareForReuse() override {  }

    void cacheForReuse() override { }
    void setStatus(MediaStatus status) {
        switch (status) {
            case MediaStatus::Pending:
                svgIcon->setImageFromSVGRes("icon/icon-pending.svg");
                break;
            case MediaStatus::Processing:
                svgIcon->setImageFromSVGRes("icon/icon-pending.svg");
                break;
            case MediaStatus::PartiallyAvailable:
                svgIcon->setImageFromSVGRes("icon/icon-available.svg");
                break;
            case MediaStatus::Available:
                svgIcon->setImageFromSVGRes("icon/icon-available.svg");
                break;
            default:
                svgIcon->setVisibility(brls::Visibility::GONE);
        }
    }

    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelExt, "video/card/label/ext");
    BRLS_BIND(LabelBackground, labelBackground, "video/card/type/background");
    BRLS_BIND(SVGImage, svgIcon, "video/card/svg/icon");
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