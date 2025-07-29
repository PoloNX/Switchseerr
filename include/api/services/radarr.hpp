#pragma once

#include <borealis.hpp>

struct RadarrService {
    int id;
    std::string name;
    bool is4k;
    bool isDefault;
    std::string activeDirectory;
    int activeProfileId;
};

struct RootFolder {
    int id;
    int freeSpace;
    std::string path;
};

struct QualityProfile {
    RadarrService radarrService;
    int id;
    std::string name;
    std::vector<RootFolder> rootFolders;
};