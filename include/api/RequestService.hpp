#pragma once

#include "http/HttpClient.hpp"
#include "auth/AuthService.hpp"
#include "models/RequestModel.hpp"
#include "models/RequestStatus.hpp"
#include <vector>
#include <optional>

class RequestService {
public:
    RequestService(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<AuthService> authService);

    bool createRequest(const MovieRequest& request, MediaType mediaType);
    
    std::vector<MovieRequest> getUserRequests();
    std::optional<MovieRequest> getRequestById(int requestId);
    
    std::vector<MovieRequest> getAllRequests();

private:
    std::shared_ptr<HttpClient> client;
    std::shared_ptr<AuthService> auth;
    
    static constexpr const char* BASE_REQUEST_URL = "/api/v1/request";
    
    std::optional<MovieRequest> parseRequestFromJson(const std::string& json) const;
    std::vector<MovieRequest> parseRequestsFromJson(const std::string& json) const;
};