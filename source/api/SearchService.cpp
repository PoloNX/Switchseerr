#include "api/SearchService.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

template<typename T>
T get_or_default(const nlohmann::json& j, const std::string& key, const T& defaultValue) {
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j.at(key).get<T>();
        } catch (...) {
            // Si mauvais type ou autre problème, on tombe ici
            return defaultValue;
        }
    }
    return defaultValue;
}

using json = nlohmann::json;

SearchService::SearchService(HttpClient& httpClient, AuthService& authService)
    : client(httpClient), auth(authService) {}

std::vector<MediaSearchResult> SearchService::search(const std::string& query) {
    std::vector<MediaSearchResult> results;
    
    //use curl_easy_escape to escape the query string
    std::string escapedQuery = curl_easy_escape(client.getCurl(), query.c_str(), query.length());

    std::string url = "http://jellyseerr.cabanaflix.ovh/api/v1/search?query=" + escapedQuery;

    std::string response = client.get(url);

    json j = json::parse(response);
    if (j.at("totalResults") == 0) return results;

    for (const auto& item : j["results"]) {
        try {
            std::string mediaType = get_or_default<std::string>(item, "mediaType", "");

            if (mediaType == "movie") {
                Movie m;
                m.id = get_or_default<int>(item, "id", 0);
                m.title = get_or_default<std::string>(item, "title", "");
                m.overview = get_or_default<std::string>(item, "overview", "");
                m.posterPath = get_or_default<std::string>(item, "posterPath", "");
                m.backdropPath = get_or_default<std::string>(item, "backdropPath", "");
                m.releaseDate = get_or_default<std::string>(item, "releaseDate", "");
                m.voteAverage = get_or_default<double>(item, "voteAverage", 0.0);

                results.push_back(MediaSearchResult{MediaType::Movie, m, {}});
            } else if (mediaType == "tv") {
                TvShow tv;
                tv.id = get_or_default<int>(item, "id", 0);
                tv.name = get_or_default<std::string>(item, "name", "");
                tv.overview = get_or_default<std::string>(item, "overview", "");
                tv.posterPath = get_or_default<std::string>(item, "posterPath", "");
                tv.backdropPath = get_or_default<std::string>(item, "backdropPath", "");
                tv.firstAirDate = get_or_default<std::string>(item, "firstAirDate", "");
                tv.voteAverage = get_or_default<double>(item, "voteAverage", 0.0);

                results.push_back(MediaSearchResult{MediaType::Tv, {}, tv});
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON error in search result item: " << e.what() << std::endl;
            continue;
        }
    }




    return results;
}
