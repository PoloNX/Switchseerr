#include <stdexcept>
#include <cstring>
#include <algorithm>
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
}

std::string HttpClient::get(const std::string& url, curl_slist* customHeaders, bool verbose) {
    brls::Logger::debug("HttpClient: GET request to {}", url);
    if (customHeaders) {
        brls::Logger::debug("HttpClient: Custom headers provided : {}", customHeaders->data);
    } else {
        brls::Logger::debug("HttpClient: No custom headers provided");
    }
    
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (customHeaders) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, customHeaders);
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Switchseerr/1.0");

    auto res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
       brls::Logger::error("CURL GET request failed: {}", std::string(curl_easy_strerror(res)));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            brls::Logger::error("CURL GET request returned non-200 status code: {}", std::to_string(response_code));
        }
    }

    return response;
}

std::string HttpClient::post(const std::string& url, const std::string& data) {
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Switchseerr/1.0");

    auto res = curl_easy_perform(curl);

    curl_slist_free_all(headers);

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

HttpClient::~HttpClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    if (headers) {
        curl_slist_free_all(headers);
    }
}

std::vector<unsigned char> HttpClient::downloadImageToBuffer(const std::string& url, bool verbose) {
    brls::Logger::debug("HttpClient: Downloading image from {} to buffer", url);
    
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

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

    brls::Logger::debug("Image downloaded successfully to buffer, size: {} bytes", buffer.size());
    return buffer;
}

// ...existing code...

size_t HttpClient::writeBufferCallback(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* buffer) {
    size_t totalSize = size * nmemb;
    unsigned char* data = static_cast<unsigned char*>(contents);
    buffer->insert(buffer->end(), data, data + totalSize);
    return totalSize;
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