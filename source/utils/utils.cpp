#include "utils/utils.hpp"

//Return date under the format YYYY-MM-DD
std::string get_date_string() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    char buffer[11]; // YYYY-MM-DD
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", now_tm);
    return std::string(buffer);
}

// template<typename T>
// T get_or_default(const nlohmann::json& j, const std::string& key, const T& defaultValue) {
//     if (j.contains(key) && !j[key].is_null()) {
//         try {
//             return j.at(key).get<T>();
//         } catch (...) {
//             return defaultValue;
//         }
//     }
//     return defaultValue;
// }

std::string format_date(const std::string& date) {
    if (date.length() != 10) {
        brls::Logger::error("Invalid date format: {}", date);
        return date; // Return original if format is incorrect
    }
    return date.substr(8, 2) + "/" + date.substr(5, 2) + "/" + date.substr(0, 4);
}

std::string escaped_string(const std::string& str) {
    std::string escaped;
    CURL* curl = curl_easy_init();
    if (curl) {
        char* escapedStr = curl_easy_escape(curl, str.c_str(), str.length());
        if (escapedStr) {
            escaped = std::string(escapedStr);
            curl_free(escapedStr);
        } else {
            brls::Logger::error("Failed to escape string: {}", str);
        }
        curl_easy_cleanup(curl);
    } else {
        brls::Logger::error("CURL initialization failed");
    }
    return escaped;
}