#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <borealis/core/logger.hpp>

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