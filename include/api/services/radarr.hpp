#pragma once

#include <borealis.hpp>

struct RootFolder {
    int id;
    int freeSpace;
    std::string path;
};

struct QualityProfile {
    int id;
    std::string name;
    std::vector<RootFolder> rootFolders;
};

struct RadarrService {
    int id;
    std::string name;
    bool is4k;
    bool isDefault;
    std::string activeDirectory;
    int activeProfileId;
    std::vector<QualityProfile> qualityProfiles;
};


