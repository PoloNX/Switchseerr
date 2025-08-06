#pragma once

#include "api/services/baseService.hpp"
#include "models/RequestModel.hpp"
class SonarrService : public BaseService {
public:
    SonarrService() = default;
    SonarrService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService, int id, const std::string& name, bool is4k, bool isDefault, const std::string& activeDirectory, int activeProfileId, const std::vector<QualityProfile>& qualityProfiles)
        : BaseService{httpClient, authService, name, id, is4k, isDefault, activeDirectory, activeProfileId, qualityProfiles} {}
    SonarrService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService)
        : BaseService{httpClient, authService} {}

    bool performRequest(const TvRequest& request);
};