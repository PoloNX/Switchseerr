#pragma once

#include <string>
#include "MediaSearchResult.hpp"

enum class DiscoverType {
    RecentlyAdded,
    RecentlyRequested,
    Trending,
    PopularMovies,
    FutureMovies,
    PopularTvShows,
    FutureTvShows
};

enum class MediaStatus {
    Unknown = 1,
    Pending,
    Processing,
    PartiallyAvailable,
    Available,
    Blacklisted,
    Deleted
};

struct MediaItem {
    int id;
    MediaType type;
    MediaStatus status;
    std::string title;     
    std::string overview;
    std::string posterPath;
    std::string backdropPath;
    std::string releaseDate;   
    std::string firstAirDate;  
    double voteAverage;
    
    MediaItem() = default;
    
    MediaItem(const Movie& movie) 
        : id(movie.id)
        , type(MediaType::Movie)
        , title(movie.title)
        , overview(movie.overview)
        , posterPath(movie.posterPath)
        , backdropPath(movie.backdropPath)
        , releaseDate(movie.releaseDate)
        , voteAverage(movie.voteAverage) {}
    
    MediaItem(const TvShow& tvShow)
        : id(tvShow.id)
        , type(MediaType::Tv)
        , title(tvShow.name)
        , overview(tvShow.overview)
        , posterPath(tvShow.posterPath)
        , backdropPath(tvShow.backdropPath)
        , firstAirDate(tvShow.firstAirDate)
        , voteAverage(tvShow.voteAverage) {}
};