#include "utils/Config.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <borealis.hpp>
#include <fstream>

#if defined(_WIN32)
#include <shlobj.h>
#endif

namespace fs = std::filesystem;

bool Config::init() {
    const std::string path = this->configDir() + "/config.json";
    brls::Logger::debug("Config: loading from {}", path);
    std::ifstream f(path);
    brls::Logger::debug("Config: loaded from {}", path);
    if (f.is_open()) {
        try {
            nlohmann::json::parse(f).get_to(*this);
            brls::Logger::info("Config: loaded from {}", path);
        } catch (const nlohmann::json::parse_error& e) {
            brls::Logger::error("Config: JSON parse error: {}", e.what());
            return false;
        }
    }
    // Ajout du return false si le fichier ne peut pas être ouvert
    brls::Logger::info("Config: init : {}", path);
    return true;
}

void Config::save() {
    try {
        std::string dir = this->configDir();
        fs::create_directories(dir);
        std::ofstream f(dir + "/config.json");
        if (f.is_open()) {
            nlohmann::json j(*this);
            f << j.dump(4);
            f.close();
        }
    } catch (const std::exception& e) {
            brls::Logger::error("Config: failed to save config: {}", e.what());
    }
}

std::string Config::configDir() {
#ifdef __linux__
    char* config_home = getenv("XDG_CONFIG_HOME");
    if (config_home) return fmt::format("{}/{}", config_home, "switchseerr");
    return fmt::format("{}/.config/switchseerr", getenv("HOME"));
#elif _WIN32
    WCHAR wpath[MAX_PATH];
    std::vector<char> lpath(MAX_PATH);
    SHGetSpecialFolderPathW(0, wpath, CSIDL_LOCAL_APPDATA, false);
    WideCharToMultiByte(CP_UTF8, 0, wpath, std::wcslen(wpath), lpath.data(), lpath.size(), nullptr, nullptr);
    return fmt::format("{}\\{}", lpath.data(), "switchseerr");
#endif
}

bool Config::addServer(const AppServer& server) {
    this->server_url = server.url;

    for (auto& o : this->servers) {
        if (server.url == o.url) {
            if (!server.name.empty()) o.name = server.name;
            if (!server.version.empty()) o.version = server.version;
            if (!server.url.empty()) o.url = server.url;
            this->save();
            return true;
        }
    }
    this->servers.push_back(server);
    this->save();
    return false;
}

void Config::addUser(const AppUser& u, const std::string& url) {
    auto is_user = [u](const AppUser& o) { return o.id == u.id; };
    auto it = std::find_if(this->users.begin(), this->users.end(), is_user);
    if (it != this->users.end()) {
        it->name = u.name;
        it->api_key = u.api_key;
        it->server_url = u.server_url;
    } else {
        it = this->users.insert(it, u);
    }
    this->server_url = url;
    this->user_id = u.id;
    this->user = it;
    this->save();
}

bool Config::removeServer(const std::string& url) {
    for (auto it = this->servers.begin(); it != this->servers.end(); ++it) {
        if (it->url == url) {
            this->servers.erase(it);
            this->save();
            return this->servers.empty();
        }
    }
    return false;
}

bool Config::removeUser(const int& id) {
    for (auto it = this->users.begin(); it != this->users.end(); ++it) {
        if (it->id == id) {
            this->users.erase(it);
            this->save();
            return true;
        }
    }
    return false;
}

const std::vector<AppUser> Config::getUsers(const std::string& url) const {
    std::vector<AppUser> users;
    for (auto& u : this->users) {
        if (u.server_url == url) {
            users.push_back(u);
        }
    }
    return users;
}