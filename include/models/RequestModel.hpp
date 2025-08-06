#pragma once

#include <vector>

#include "models/Movie.hpp"
#include "models/MediaSearchResult.hpp"

struct MovieRequest {
    MediaType type;
    int mediaId;
    int tvdbId;
    bool is4k;
    int serverId;
    int profileId;
    std::string rootFolder;
    int languageProfileId;
    int userId;
};

struct TvRequest {
    MediaType type;
    int mediaId;
    int tvdbId;
    bool is4k;
    int serverId;
    int profileId;
    std::string rootFolder;
    int languageProfileId;
    int userId;
    std::vector<int> selectedSeasons;
};