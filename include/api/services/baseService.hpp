#pragma once

#include <string>
#include <vector>
#include <memory>

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/MediaItem.hpp"

struct RootFolder {
    int id;
    int freeSpace;
    std::string path;
};

struct QualityProfile {
    int id;
    std::string name;
    std::vector<RootFolder> rootFolders;
    bool defaultProfile;
};

class BaseService {
public:
    BaseService() = default;
    BaseService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, const std::string& serviceName, int id, bool is4k, bool isDefault, const std::string& activeDirectory, int activeProfileId, const std::vector<QualityProfile>& qualityProfiles)
        : httpClient(httpClient), authService(authService), serverUrl(authService->getServerUrl()), serviceName(serviceName), id(id), is4k_(is4k), isDefault_(isDefault), activeDirectory(activeDirectory), activeProfileId(activeProfileId), qualityProfiles(qualityProfiles) {}
    BaseService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
        : httpClient(httpClient), authService(authService), serverUrl(authService->getServerUrl()) {}
    virtual ~BaseService() = default;

    // getters
    const std::string& getServiceName() const { return serviceName; }
    int getId() const { return id; }
    bool is4k() const { return is4k_; }
    bool isDefault() const { return isDefault_; }
    const std::string& getActiveDirectory() const { return activeDirectory; }
    int getActiveProfileId() const { return activeProfileId; }
    const std::vector<QualityProfile>& getQualityProfiles() const { return qualityProfiles; }

    // setterss
    void setServiceName(const std::string& name) { serviceName = name; }
    void setId(int serviceId) { id = serviceId; }
    void setIs4k(bool value) { is4k_ = value; }
    void setIsDefault(bool value) { isDefault_ = value; }
    void setActiveDirectory(const std::string& directory) { activeDirectory = directory; }
    void setActiveProfileId(int profileId) { activeProfileId = profileId; }
    void setQualityProfiles(const std::vector<QualityProfile>& profiles) { qualityProfiles = profiles; }

protected:

    std::string serverUrl;

    std::shared_ptr<HttpClient> httpClient;
    std::shared_ptr<AuthService> authService;

    std::string serviceName;
    int id;
    bool is4k_;
    bool isDefault_;
    std::string activeDirectory;
    int activeProfileId;
    std::vector<QualityProfile> qualityProfiles;
};