#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::string get(const std::string& url, struct curl_slist* headers = nullptr, bool versbose = false);

    std::string post(const std::string& url, const std::string& data);

    std::vector<unsigned char> downloadImageToBuffer(const std::string& url, bool verbose = false);

    CURL* getCurl() const {
        return curl;
    }
private:
    CURL* curl;
    struct curl_slist* headers;

    static size_t writeBufferCallback(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* buffer);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* buffer);
    static size_t readCallback(void* contents, size_t size, size_t nmemb, std::string* buffer);
};