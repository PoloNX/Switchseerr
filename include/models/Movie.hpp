#pragma once
#include <string>

struct Movie {
    int id;
    std::string title;
    std::string overview;
    std::string posterPath;
    std::string backdropPath;
    std::string releaseDate;
    double voteAverage;
};
