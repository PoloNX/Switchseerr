#pragma once

#include <string>
#include <vector>
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
    Pending = 2,
    Processing = 3,
    PartiallyAvailable = 4,
    Available = 5,
    Blacklisted = 6,
    Deleted = 7
};

struct Season {
    std::string airDate;
    int episodeCount;
    int id;
    std::string name;
    std::string overview;
    int seasonNumber;
    std::string posterPath;
};

struct MediaItem {
    //common
    int id;
    MediaType type;
    MediaStatus status;
    std::string title;     
    std::string overview;
    std::string posterPath;
    std::string backdropPath;
    std::string originalLanguage;
    std::vector<std::string> genres;
    
    //movie specific
    std::string releaseDate;  
    std::string originalTitle;
    int revenue;
    int runtime;
    std::string statusString;

    //tv specific
    std::string firstAirDate;
    bool inProduction;
    std::string lasAirDate;
    int numberOfEpisodes;
    int numberOfSeasons;
    std::string originalName;
    std::vector<Season> seasons;


    double voteAverage;
};