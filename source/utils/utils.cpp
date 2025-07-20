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