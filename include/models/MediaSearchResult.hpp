#pragma once

#include "Movie.hpp"
#include "TvShow.hpp"

enum class MediaType {
    Movie,
    Tv
};

inline std::string mediaTypeToString(MediaType type) {
    switch (type) {
        case MediaType::Movie:
            return "movie";
        case MediaType::Tv:
            return "tv";
        default:
            return "movie"; // valeur par défaut
    }
}

struct MediaSearchResult {
    MediaType type;
    Movie movie;  
    TvShow tvShow;   
};
