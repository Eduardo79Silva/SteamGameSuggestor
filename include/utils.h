#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif // CURL_STATICLIB
#pragma once


#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

std::string unixTimeToHumanReadable(long int seconds);

std::string removeSpecialCharacters(const std::string &input);

