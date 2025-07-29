#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <borealis/core/singleton.hpp>

struct AppUser {
    int id;
    std::string name;
    std::string api_key;
    std::string server_url;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppUser, id, name, api_key, server_url);

struct AppServer {        
    std::string name;
    std::string version;
    std::string url;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AppServer ,name, version, url);

class Config : public brls::Singleton<Config> {
    using UserIter = std::vector<AppUser>::iterator;

public:
    Config() = default;

    bool init();
    void save();
    bool checkLogin();
    
    std::string configDir();
    
    struct Option {
        std::string key;
        std::vector<std::string> options;
        std::vector<long> values;
    };

    bool addServer(const AppServer& server);
    void addUser(const AppUser& user, const std::string& url);
    bool removeServer(const std::string& url);
    bool removeUser(const int& id);
    const std::string getUserName() const { return this->user->name;} 
    const std::string getUrl() const { return this->server_url; }
    const std::vector<AppServer>& getServers() const { return this->servers; }
    const std::vector<AppUser> getUsers(const std::string& id) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Config, user_id, users, servers, setting);

private:
    UserIter user;
    std::string user_id;
    std::string server_url;
    std::vector<AppUser> users;
    std::vector<AppServer> servers;
    nlohmann::json setting = {};
};