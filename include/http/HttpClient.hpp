#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::string get(const std::string& url, struct curl_slist* headers = nullptr, bool verbose = false);

    std::string post(const std::string& url, const std::string& data, struct curl_slist* headers = nullptr, bool verbose = false);

    std::vector<unsigned char> downloadImageToBuffer(const std::string& url, bool verbose = false);

    CURL* getCurl() const {
        return curl;
    }
private:
    CURL* curl;
    curl_slist* headers;
    curl_slist* defaultHeaders; 

    void initDefaultHeaders(); 
    curl_slist* mergeHeaders(curl_slist* customHeaders); 

    static size_t writeBufferCallback(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* buffer);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* buffer);
    static size_t readCallback(void* contents, size_t size, size_t nmemb, std::string* buffer);
};