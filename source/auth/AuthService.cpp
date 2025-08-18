#include "auth/AuthService.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fmt/format.h>
#include <borealis.hpp>
#include <filesystem>

AuthService::AuthService(std::shared_ptr<HttpClient> client, const std::string& serverUrl) : client(client), serverUrl(serverUrl) {}

bool AuthService::loginWithJellyfin(const std::string& username, const std::string& password) {
    brls::Logger::debug("AuthService: Attempting login with jellyfin for user: {}", username);

    setupCookieFileForUser(username);

        std::string cookiePath = getCookieFilePath(username);
    brls::Logger::debug("AuthService: Cookie file path: {}", cookiePath);
    

    std::string loginUrl = fmt::format("{}/api/v1/auth/jellyfin", serverUrl); // Use the provided server URL

    nlohmann::json body = {
        {"username", username},
        {"password", password}
    };

    try {
        // Authenticate user with proper headers
        const std::string response = client->post(loginUrl, body.dump());
        const auto loginData = nlohmann::json::parse(response);
        
        if (!loginData.contains("id")) {
            brls::Logger::error("AuthService: Login response missing user ID");
            return false;
        }

        // Login successful, extract user info
        const int userId = loginData["id"].get<int>();
        brls::Logger::debug("AuthService: Login successful, user ID: {}", userId);

        
        currentUser = {
            .id = userId,
            .name = loginData["displayName"].get<std::string>(),
            .server_url = serverUrl
        };

        Config::instance().addUser(currentUser, serverUrl);

        client->flushCookies();

                // Vérifier si le cookie a été créé
        if (client->hasCookies()) {
            brls::Logger::info("AuthService: Cookies successfully saved for user {}", username);
            // Afficher le contenu du fichier de cookies pour debug
            std::ifstream cookieFile(cookiePath);
            if (cookieFile.is_open()) {
                std::string line;
                brls::Logger::debug("AuthService: Cookie file contents:");
                while (std::getline(cookieFile, line)) {
                    brls::Logger::debug("  - {}", line);
                }
                cookieFile.close();
            } else {
                brls::Logger::error("AuthService: Failed to open cookie file: {}", cookiePath);
            }
        } else {
            brls::Logger::warning("AuthService: No cookies were saved for user {}", username);
            // Lister le contenu du répertoire pour debug
            std::filesystem::path configPath = Config::instance().configDir();
            brls::Logger::debug("AuthService: Config directory contents:");
            for (const auto& entry : std::filesystem::directory_iterator(configPath)) {
                brls::Logger::debug("  - {}", entry.path().filename().string());
            }
        }

        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("AuthService: JSON parsing failed: {}", e.what());
    } catch (const nlohmann::json::type_error& e) {
        brls::Logger::error("AuthService: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        brls::Logger::error("AuthService: Login failed: {}", e.what());
    }

    return false;
}

bool AuthService::loginWithPlex() {
    brls::Logger::debug("AuthService: Attempting login with Plex token");
    // todo : implement Plex login logic
}

bool AuthService::loginWithLocal(const std::string& email, const std::string& password) {
    brls::Logger::debug("AuthService: Attempting login with local for user: {}", email);

        setupCookieFileForUser(email);

    std::string loginUrl = fmt::format("{}/api/v1/auth/local", serverUrl); // Use the provided server URL

    nlohmann::json body = {
        {"email", email},
        {"password", password}
    };

    try {
        // Authenticate user with proper headers
        const std::string response = client->post(loginUrl, body.dump());
        const auto loginData = nlohmann::json::parse(response);
        
        if (!loginData.contains("id")) {
            brls::Logger::error("AuthService: Login response missing user ID");
            return false;
        }

        // Login successful, extract user info
        const int userId = loginData["id"].get<int>();
        brls::Logger::debug("AuthService: Login successful, user ID: {}", userId);

        
        currentUser = {
            .id = userId,
            .name = loginData["displayName"].get<std::string>(),
            .server_url = serverUrl
        };

        Config::instance().addUser(currentUser, serverUrl);



        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("AuthService: JSON parsing failed: {}", e.what());
    } catch (const nlohmann::json::type_error& e) {
        brls::Logger::error("AuthService: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        brls::Logger::error("AuthService: Login failed: {}", e.what());
    }

    return false;
}


std::string AuthService::getCookieFilePath(const std::string& username) const {
    // Créer un nom de fichier unique basé sur l'URL du serveur ET le nom d'utilisateur
    std::string serverHash = std::to_string(std::hash<std::string>{}(serverUrl));
    std::string userHash = std::to_string(std::hash<std::string>{}(username));
    std::string cookieFileName = fmt::format("jellyseerr_cookies_{}_{}.txt", serverHash, userHash);
    return fmt::format("{}/{}", Config::instance().configDir(), cookieFileName);
}

void AuthService::setupCookieFileForUser(const std::string& username) {
    std::string cookieFilePath = getCookieFilePath(username);
    client->setCookieFile(cookieFilePath);
    brls::Logger::debug("AuthService: Cookie file configured for user {}: {}", username, cookieFilePath);
}

bool AuthService::tryLoginFromCookies(const std::string& username) {
    brls::Logger::debug("AuthService: Attempting login from saved cookies for user: {}", username);
    
    // Configurer le fichier de cookies pour cet utilisateur
    setupCookieFileForUser(username);
    
    // Vérifier si un fichier de cookies existe pour cet utilisateur
    if (!client->hasCookies()) {
        brls::Logger::debug("AuthService: No saved cookies found for user: {}", username);
        return false;
    }
    
    try {
        // Tenter une requête qui nécessite une authentification
        std::string userInfoUrl = fmt::format("{}/api/v1/auth/me", serverUrl);
        std::string response = client->get(userInfoUrl);
        
        // Parser la réponse pour vérifier si l'authentification a réussi
        auto userData = nlohmann::json::parse(response);
        
        if (!userData.contains("id")) {
            brls::Logger::debug("AuthService: Cookie authentication failed for user {} - invalid response", username);
            return false;
        }

        int permission = userData["permissions"].get<int>();
        extractPermissions(permission);

        // Extraire les informations utilisateur
        const int userId = userData["id"].get<int>();
        std::string displayName = userData["displayName"].get<std::string>();
    
        
        brls::Logger::debug("AuthService: Cookie authentication successful for user {}, ID: {}", username, userId);
        
        // Mettre à jour currentUser
        currentUser = {
            .id = userId,
            .name = displayName,
            .server_url = serverUrl
        };
        
        brls::Logger::info("AuthService: Successfully logged in as {} using saved cookies", currentUser.name);
        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        brls::Logger::error("AuthService: JSON parsing failed during cookie login for user {}: {}", username, e.what());
    } catch (const nlohmann::json::type_error& e) {
        brls::Logger::error("AuthService: JSON type error during cookie login for user {}: {}", username, e.what());
    } catch (const std::exception& e) {
        brls::Logger::warning("AuthService: Cookie authentication failed for user {}: {}", username, e.what());
        // Les cookies sont probablement expirés, les nettoyer
        client->clearCookies();
    }
    
    return false;
}


void AuthService::logout() {
    brls::Logger::debug("AuthService: Logging out user: {}", currentUser.name);
    
    // Nettoyer les cookies pour l'utilisateur actuel
    client->clearCookies();
    
    // Réinitialiser l'utilisateur actuel
    currentUser = {};
    
    brls::Logger::info("AuthService: Logged out successfully");
}

bool AuthService::isLoggedIn() {
    // Vérifier si nous avons un utilisateur en mémoire
    return currentUser.id != 0;
}

void AuthService::extractPermissions(int permissionValue) {
    userPermissions.clear();
    advancedRequest = false;

    struct Perm {
        int value;
        JellyseerrPermission perm;
    };

    std::vector<Perm> allPerms = {
        {2, JellyseerrPermission::ADMIN},
        {4, JellyseerrPermission::MANAGE_SETTINGS},
        {8, JellyseerrPermission::MANAGE_USERS},
        {16, JellyseerrPermission::MANAGE_REQUESTS},
        {32, JellyseerrPermission::REQUEST},
        {64, JellyseerrPermission::VOTE},
        {128, JellyseerrPermission::AUTO_APPROVE},
        {256, JellyseerrPermission::AUTO_APPROVE_MOVIE},
        {512, JellyseerrPermission::AUTO_APPROVE_TV},
        {1024, JellyseerrPermission::REQUEST_4K},
        {2048, JellyseerrPermission::REQUEST_4K_MOVIE},
        {4096, JellyseerrPermission::REQUEST_4K_TV},
        {8192, JellyseerrPermission::REQUEST_ADVANCED},
        {16384, JellyseerrPermission::REQUEST_VIEW},
        {32768, JellyseerrPermission::AUTO_APPROVE_4K},
        {65536, JellyseerrPermission::AUTO_APPROVE_4K_MOVIE},
        {131072, JellyseerrPermission::AUTO_APPROVE_4K_TV},
        {262144, JellyseerrPermission::REQUEST_MOVIE},
        {524288, JellyseerrPermission::REQUEST_TV},
        {1048576, JellyseerrPermission::MANAGE_ISSUES},
        {2097152, JellyseerrPermission::VIEW_ISSUES},
        {4194304, JellyseerrPermission::CREATE_ISSUES},
        {8388608, JellyseerrPermission::AUTO_REQUEST},
        {16777216, JellyseerrPermission::AUTO_REQUEST_MOVIE},
        {33554432, JellyseerrPermission::AUTO_REQUEST_TV},
        {67108864, JellyseerrPermission::RECENT_VIEW},
        {134217728, JellyseerrPermission::WATCHLIST_VIEW},
        {268435456, JellyseerrPermission::MANAGE_BLACKLIST},
        {1073741824, JellyseerrPermission::VIEW_BLACKLIST},
    };

    for (const auto& p : allPerms) {
        if (permissionValue == 2) {
            userPermissions.push_back(p.perm);
            advancedRequest = true; // If the user has ADMIN permission, they have all permissions
        }
        else if (permissionValue & p.value) {
            userPermissions.push_back(p.perm);
            if (p.perm == JellyseerrPermission::REQUEST_ADVANCED) {
                advancedRequest = true;
            }
        }
    }

    // logs
    std::string all_perms = "All Permissions: ";
    for (const auto& perm : userPermissions) {
        all_perms += std::to_string(static_cast<int>(perm)) + ", ";
    }
    all_perms = all_perms.substr(0, all_perms.size() - 2);  // Remove trailing comma and space
    brls::Logger::info("AuthService: user perms {}", all_perms);
    brls::Logger::debug("AuthService: advancedRequest {}", advancedRequest);

    if (userPermissions.empty()) {
        userPermissions.push_back(JellyseerrPermission::NONE);
    }
}

