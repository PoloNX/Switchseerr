#pragma once

#include <vector>

#include "models/Movie.hpp"
#include "models/MediaSearchResult.hpp"

struct MovieRequest {
    MediaType type;
    int mediaId;
    int tvdbId;
    std::vector<int> seasons;
    bool is4k;
    int serverId;
    int profilerId;
    std::string rootFolder;
    int languageProfileId;
    int userId;
};