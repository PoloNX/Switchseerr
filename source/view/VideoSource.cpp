#include "view/VideoSource.hpp"
#include "view/VideoCard.hpp"
#include <mutex>

static std::mutex downloadMutex;


VideoDataSource::VideoDataSource(const MediaList& r, HttpClient& httpClient, bool resume)
    : list(r), resume(resume), httpClient(httpClient) { brls::Logger::debug("VideoDataSource", "Creating VideoDataSource with {} items", list.size()); } 

size_t VideoDataSource::getItemCount() {
    return list.size();
}

RecyclingGridItem* VideoDataSource::cellForRow(RecyclingView* recycler, size_t index) {
    brls::Logger::debug("VideoDataSource, Creating cell for item at index: {}", index);
    VideoCardCell* cell = dynamic_cast<VideoCardCell*>(recycler->dequeueReusableCell("Cell"));
    auto& item = this->list.at(index);
    cell->setId(std::to_string(item.id));

    if (item.type == MediaType::Tv) {
        if(item.title.empty()) {
            cell->labelTitle->setVisibility(brls::Visibility::GONE);
        } else {
            cell->labelTitle->setText(item.title);
        }
        cell->labelExt->setText(item.firstAirDate);
    } else {
        cell->labelTitle->setText(item.title);
        cell->labelExt->setText(item.releaseDate);
    }


    //Picture downloading
    if (!item.posterPath.empty()) {
        std::thread([this, item, cell]() {
            brls::Logger::debug("VideoDataSource, Downloading image for item ID: {}", item.title);
            
            std::vector<unsigned char> imageBuffer;
            {
                std::lock_guard<std::mutex> lock(downloadMutex);
                imageBuffer = this->httpClient.downloadImageToBuffer(fmt::format("https://image.tmdb.org/t/p/w300_and_h450_face{}", item.backdropPath));
            }
            
            if(!imageBuffer.empty()) {
                brls::sync([cell, item, imageBuffer = std::move(imageBuffer)] {
                    brls::Logger::debug("VideoDataSource, Image downloaded successfully for item ID: {}", item.id);
                    cell->picture->setImageFromMem(imageBuffer.data(), imageBuffer.size());
                });
            } else {
                brls::Logger::error("VideoDataSource, Failed to download image for item ID: {}", item.id);
            }
        }).detach();
    }


    return cell;
}

void VideoDataSource::onItemSelected(brls::Box* recycler, size_t index) {
    auto& item = this->list.at(index);
    brls::Logger::debug("VideoDataSource: Item selected with ID: {}", item.id);
    
    // Handle item selection logic here, e.g., navigate to details view
}

void VideoDataSource::clearData() {
    this->list.clear();
}