#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <borealis/core/logger.hpp>

#include <chrono>

template<typename T>
T get_or_default(const nlohmann::json& j, const std::string& key, const T& defaultValue) {
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j[key].get<T>();
        } catch (const std::exception& e) {
            brls::Logger::warning("Jellyseerr: Error getting key '{}' from JSON: {}", key, e.what());
            return defaultValue;
        }
    }
    return defaultValue;
}
//Return date under the format YYYY-MM-DD
std::string get_date_string();