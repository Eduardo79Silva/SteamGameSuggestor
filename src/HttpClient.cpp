#include "HttpClient.h"
#include <iostream>

// Static callback function for writing response data
size_t HttpClient::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userData) {
    size_t totalSize = size * nmemb;
    userData->append((char *)contents, totalSize);
    return totalSize;
}

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_ALL);
    handle = curl_easy_init();
    if (!handle) {
        throw std::runtime_error("Failed to create CURL handle");
    }
}

HttpClient::~HttpClient() {
    if (handle) {
        curl_easy_cleanup(handle);
    }
    curl_global_cleanup();
}

nlohmann::json HttpClient::callAPI(const std::string &url) {
    std::string readBuffer;
    curl_easy_reset(handle); // Reset options from previous use
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 0L);
    
    CURLcode res = curl_easy_perform(handle);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    auto data = nlohmann::json::parse(readBuffer, nullptr, false);
    return data.is_discarded() ? nlohmann::json() : data;
}

