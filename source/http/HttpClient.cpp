#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <borealis/core/logger.hpp>

#include "http/HttpClient.hpp"

HttpClient::HttpClient() {
    curl = curl_easy_init();
    headers = nullptr;
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    initDefaultHeaders();
}

HttpClient::~HttpClient() {
    brls::Logger::debug("HttpClient: Cleaning up CURL resources");
    if (curl) {
        curl_easy_cleanup(curl);
    }
    if (headers) {
        curl_slist_free_all(headers);
    }
    if (defaultHeaders) {
        curl_slist_free_all(defaultHeaders);
    }
}

void HttpClient::setCookieFile(const std::string& cookieFilePath) {
    this->cookieFilePath = cookieFilePath;
    
    if (!cookieFilePath.empty()) {
        // Créer le répertoire parent si nécessaire
        std::filesystem::path path(cookieFilePath);
        std::filesystem::create_directories(path.parent_path());
        
        // Configurer CURL pour utiliser le fichier de cookies
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFilePath.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFilePath.c_str());

        // Activer explicitement le moteur de cookies
        curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 0L);
        
        brls::Logger::debug("HttpClient: Cookie file set to {}", cookieFilePath);
    } else {
        // Réinitialiser aux valeurs par défaut
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");
        brls::Logger::debug("HttpClient: Cookie file cleared");
    }
}

void HttpClient::clearCookies() {
    if (!cookieFilePath.empty()) {
        try {
            // Vider les cookies en mémoire dans CURL
            curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
            
            // Supprimer le fichier de cookies
            if (std::filesystem::exists(cookieFilePath)) {
                std::filesystem::remove(cookieFilePath);
                brls::Logger::debug("HttpClient: Cleared cookies file: {}", cookieFilePath);
            }
        } catch (const std::exception& e) {
            brls::Logger::error("HttpClient: Failed to clear cookies: {}", e.what());
        }
    } else {
        // Juste vider les cookies en mémoire
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
        brls::Logger::debug("HttpClient: Cleared cookies from memory");
    }
}

void HttpClient::flushCookies() {
    if (!cookieFilePath.empty() && curl) {
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, "FLUSH");
        brls::Logger::debug("HttpClient: Manually flushed cookies to file: {}", cookieFilePath);
    }
}

bool HttpClient::hasCookies() {
    if (cookieFilePath.empty()) {
        return false;
    }
    
    try {
        return std::filesystem::exists(cookieFilePath) && 
               std::filesystem::file_size(cookieFilePath) > 0;
    } catch (const std::exception& e) {
        brls::Logger::error("HttpClient: Error checking cookie file: {}", e.what());
        return false;
    }
}

// ...existing code...
void HttpClient::initCookies() {
    if (!cookieFilePath.empty()) {
        brls::Logger::debug("HttpClient: Initializing cookies with file: {}", cookieFilePath);
        // Configurer le fichier de cookies
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFilePath.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFilePath.c_str());
        // Activer explicitement le moteur de cookies
        curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 0L);

        // Fallback : lire le fichier de cookies et injecter un header "Cookie" si nécessaire.
        try {
            std::ifstream cookieFile(cookieFilePath);
            // si l'ouverture échoue, essayer une conversion simple sdmc:/ -> /sdmc/
            if (!cookieFile.is_open() && cookieFilePath.rfind("sdmc:", 0) == 0) {
                std::string alt = cookieFilePath;
                alt.replace(0, 5, "/sdmc");
                cookieFile.open(alt);
            }

            if (cookieFile.is_open()) {
                std::string line;
                std::vector<std::string> cookiePairs;
                const std::string httpOnlyPrefix = "#HttpOnly_";
                while (std::getline(cookieFile, line)) {
                    if (line.empty()) continue;

                    // Handle "#HttpOnly_domain" entries: not a comment, remove the marker
                    if (line.rfind(httpOnlyPrefix, 0) == 0) {
                        line = line.substr(httpOnlyPrefix.size());
                    } else if (line[0] == '#') {
                        // Real comment line, skip
                        continue;
                    }

                    // Netscape format: domain<TAB>flag<TAB>path<TAB>secure<TAB>expiration<TAB>name<TAB>value
                    if (line.find('\t') != std::string::npos) {
                        std::vector<std::string> parts;
                        size_t start = 0;
                        while (true) {
                            size_t pos = line.find('\t', start);
                            if (pos == std::string::npos) {
                                parts.push_back(line.substr(start));
                                break;
                            }
                            parts.push_back(line.substr(start, pos - start));
                            start = pos + 1;
                        }
                        if (parts.size() >= 7) {
                            std::string name = parts[parts.size() - 2];
                            std::string value = parts[parts.size() - 1];
                            cookiePairs.push_back(name + "=" + value);
                        }
                    } else {
                        // fallback: try to find name=value tokens
                        size_t eq = line.find('=');
                        if (eq != std::string::npos) {
                            size_t end = line.find(';', eq);
                            std::string kv = (end == std::string::npos) ? line.substr(0) : line.substr(0, end);
                            size_t first = kv.find_first_not_of(" \t");
                            if (first != std::string::npos) kv = kv.substr(first);
                            cookiePairs.push_back(kv);
                        }
                    }
                }
                cookieFile.close();

                if (!cookiePairs.empty()) {
                    std::string cookieHeader;
                    for (size_t i = 0; i < cookiePairs.size(); ++i) {
                        if (i) cookieHeader += "; ";
                        cookieHeader += cookiePairs[i];
                    }
                    curl_easy_setopt(curl, CURLOPT_COOKIE, cookieHeader.c_str());
                    brls::Logger::debug("HttpClient: Injected Cookie header from file: {}", cookieHeader);
                } else {
                    brls::Logger::debug("HttpClient: No cookie pairs parsed from file");
                }
            } else {
                brls::Logger::debug("HttpClient: Could not open cookie file for manual parse: {}", cookieFilePath);
            }
        } catch (const std::exception& e) {
            brls::Logger::warning("HttpClient: Exception while reading cookie file: {}", e.what());
        }
    } else {
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    }
}
// ...existing code...

void HttpClient::initDefaultHeaders() {
    // Créer les headers par défaut
    defaultHeaders = nullptr;
    defaultHeaders = curl_slist_append(defaultHeaders, "Content-Type: application/json");
    defaultHeaders = curl_slist_append(defaultHeaders, "Accept: application/json");
    defaultHeaders = curl_slist_append(defaultHeaders, "User-Agent: Switchseerr/1.0");
}

curl_slist* HttpClient::mergeHeaders(curl_slist* customHeaders) {
    curl_slist* mergedHeaders = nullptr;
    
    // Copier les headers par défaut
    curl_slist* current = defaultHeaders;
    while (current) {
        mergedHeaders = curl_slist_append(mergedHeaders, current->data);
        current = current->next;
    }
    
    // Ajouter les headers personnalisés (ils peuvent override les defaults)
    current = customHeaders;
    while (current) {
        mergedHeaders = curl_slist_append(mergedHeaders, current->data);
        current = current->next;
    }
    
    return mergedHeaders;
}

std::string HttpClient::get(const std::string& url, curl_slist* customHeaders, bool verbose) {
    brls::Logger::info("HttpClient: GET request to {}", url);
    
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }

    curl_easy_reset(curl);
        
    initCookies();

    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_slist* finalHeaders = mergeHeaders(customHeaders);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, finalHeaders);

    std::string response;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(curl);

    curl_slist_free_all(finalHeaders);

    if (res != CURLE_OK) {
        brls::Logger::error("CURL GET request failed: {}", std::string(curl_easy_strerror(res)));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            brls::Logger::error("CURL GET request returned non-200 status code: {}", std::to_string(response_code));
            brls::Logger::error("CURL GET request returned non-200 status code: {} - Response body: {}", std::to_string(response_code), response);        }
    }

    return response;
}

std::string HttpClient::post(const std::string& url, const std::string& data, curl_slist* customHeaders, bool verbose) {
    brls::Logger::info("HttpClient: POST request to {}", url);
    brls::Logger::debug("HttpClient: Cookie file path: {}", cookieFilePath);
    
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }

    curl_easy_reset(curl);
    
    // Reconfigurer les cookies après reset
    initCookies();

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 0L);
    
    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    
    curl_slist* finalHeaders = mergeHeaders(customHeaders);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, finalHeaders);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(curl);

    curl_slist_free_all(finalHeaders);

    // Forcer l'écriture des cookies après la requête
    if (!cookieFilePath.empty()) {
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, "FLUSH");
        brls::Logger::debug("HttpClient: Forced cookie flush to file");
    }

    if (res != CURLE_OK) {
        throw std::runtime_error("CURL POST request failed: " + std::string(curl_easy_strerror(res)));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code < 200 || response_code >= 300) {
            throw std::runtime_error("CURL POST request returned status code: " + std::to_string(response_code) + " - Response: " + response);
        }
    }

    return response;
}

std::vector<unsigned char> HttpClient::downloadImageToBuffer(const std::string& url, bool verbose) {
    brls::Logger::verbose("HttpClient: Downloading image from {} to buffer", url);
    
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }
    
    curl_easy_reset(curl);
    
    // Reconfigurer les cookies après reset
    initCookies();

    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Switchseerr/1.0");

    std::vector<unsigned char> buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBufferCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    auto res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        brls::Logger::error("CURL image download failed: {}", std::string(curl_easy_strerror(res)));
        return std::vector<unsigned char>(); // Retourne un buffer vide en cas d'erreur
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            brls::Logger::error("CURL image download returned non-200 status code: {}", std::to_string(response_code));
            return std::vector<unsigned char>(); // Retourne un buffer vide en cas d'erreur
        }
    }

    brls::Logger::verbose("Image downloaded successfully to buffer, size: {} bytes", buffer.size());
    return buffer;
}

size_t HttpClient::writeBufferCallback(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* buffer) {
    if (!buffer || !contents) {
        return 0;
    }

    size_t totalSize = size * nmemb;
    if (totalSize == 0) {
        return 0; 
    }

    unsigned char* data = static_cast<unsigned char*>(contents);

    try {
        buffer->reserve(buffer->size() + totalSize);
        buffer->insert(buffer->end(), data, data + totalSize);
        return totalSize;
    }
    catch (const std::exception& e) {
        brls::Logger::error("Error in writeBufferCallback: {}", e.what());
        return 0;
    }
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
    size_t totalSize = size * nmemb;
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t HttpClient::readCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
    size_t totalSize = size * nmemb;
    if (buffer->empty()) {
        return 0; // No more data to read
    }
    size_t bytesToCopy = (std::min)(totalSize, buffer->size());
    memcpy(contents, buffer->c_str(), bytesToCopy);
    buffer->erase(0, bytesToCopy);
    return bytesToCopy;
}