#include "utils/utils.hpp"

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