#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif // CURL_STATICLIB

#include "HLTB.h"
#include <cstddef>
#include <curl/curl.h>
#include <random>
#include <regex>
#include <sstream>

namespace howlongtobeat {

// Initialize static member
std::string HTMLRequests::SEARCH_URL = "https://www.howlongtobeat.com/api/s/";

// curl callback function to write received data
size_t HTMLRequests::write_callback(char *ptr, size_t size, size_t nmemb,
                                    std::string *data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

// Utility function to convert SearchModifiers enum to string
std::string search_modifier_to_string(SearchModifiers modifier) {
  switch (modifier) {
  case SearchModifiers::NONE:
    return "";
  case SearchModifiers::ISOLATE_DLC:
    return "only_dlc";
  case SearchModifiers::ISOLATE_MODS:
    return "only_mods";
  case SearchModifiers::ISOLATE_HACKS:
    return "only_hacks";
  case SearchModifiers::HIDE_DLC:
    return "hide_dlc";
  default:
    return "";
  }
}

// SearchInformations implementation
SearchInformations::SearchInformations(const std::string &script_content) {
  api_key = extract_api_from_script(script_content);
  search_url = extract_search_url_script(script_content);

  if (HTMLRequests::BASE_URL[std::string(HTMLRequests::BASE_URL).length() -
                             1] == '/' &&
      !search_url.empty()) {
    if (search_url[0] == '/') {
      search_url = search_url.substr(1);
    }
  }
}

std::string
SearchInformations::extract_api_from_script(const std::string &script_content) {
  // Try multiple find patterns as HLTB keeps changing format

  // Test 1 - The API Key is in the user id in the request json
  std::regex user_id_api_key_pattern(
      R"regex(users\s*:\s*\{\s*id\s*:\s*"([^"]+)")regex");
  std::smatch matches;
  if (std::regex_search(script_content, matches, user_id_api_key_pattern) &&
      matches.size() > 1) {
    return matches[1].str();
  }

  // Test 2 - The API Key is in format fetch("/api/[word
  // here]/".concat("X").concat("Y")...
  std::regex concat_api_key_pattern(
      R"regex(\/api\/\w+\/"(?:\.concat\("[^"]*"\))*)regex");
  std::string key;
  if (std::regex_search(script_content, matches, concat_api_key_pattern)) {
    std::string match_str = matches[0].str();
    std::regex concat_pattern(R"regex(\.concat\("([^"]*)"\))regex");
    std::smatch concat_matches;
    std::string::const_iterator search_start(match_str.cbegin());
    while (std::regex_search(search_start, match_str.cend(), concat_matches,
                             concat_pattern)) {
      key += concat_matches[1].str();
      search_start = concat_matches.suffix().first;
    }

    if (!key.empty()) {
      return key;
    }
  }

  // Unable to find
  return "";
}

std::string SearchInformations::extract_search_url_script(
    const std::string &script_content) {
  std::regex pattern(
      R"regex(fetch\(\s*["'](/api/[^"']*)['"]\s*((?:\.concat\(\s*["']([^"']*)["']\s*\))+)\s*,)regex",
      std::regex::ECMAScript);

  std::smatch matches;
  auto search_start = script_content.cbegin();
  while (std::regex_search(search_start, script_content.cend(), matches,
                           pattern)) {
    std::string endpoint = matches[1].str();
    std::string concat_calls = matches[2].str();

    // Extract all concatenated strings
    std::regex concat_pattern(R"(\.concat\(\s*["']([^"']*)["']\s*\))");
    std::string concatenated_str;

    std::smatch concat_matches;
    std::string::const_iterator concat_search_start(concat_calls.cbegin());
    while (std::regex_search(concat_search_start, concat_calls.cend(),
                             concat_matches, concat_pattern)) {
      concatenated_str += concat_matches[1].str();
      concat_search_start = concat_matches.suffix().first;
    }

    // Check if the concatenated string matches the known api_key
    if (concatenated_str == api_key) {
      return endpoint;
    }

    search_start = matches.suffix().first;
  }

  // Unable to find
  return "";
}

// HTMLRequests implementation
std::string HTMLRequests::get_random_user_agent() {
  // List of common user agents
  static const std::vector<std::string> user_agents = {
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/91.0.4472.124 Safari/537.36",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 "
      "(KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 "
      "Firefox/89.0",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
      "(KHTML, like Gecko) Version/14.1.1 Safari/605.1.15",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/91.0.4472.124 Safari/537.36 Edg/91.0.864.59"};

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, user_agents.size() - 1);

  return user_agents[dis(gen)];
}

std::map<std::string, std::string> HTMLRequests::get_search_request_headers() {
  std::map<std::string, std::string> headers = {
      {"Content-Type", "application/json"},
      {"Accept", "*/*"},
      {"User-Agent", get_random_user_agent()},
      {"Referer", REFERER_HEADER}};
  return headers;
}

std::string HTMLRequests::get_search_request_data(
    const std::string &game_name, SearchModifiers search_modifiers, int page,
    const SearchInformations *search_info) {
  // Split game name into terms
  std::vector<std::string> search_terms;
  std::istringstream iss(game_name);
  std::string term;
  while (iss >> term) {
    search_terms.push_back(term);
  }

  nlohmann::json payload = {
      {"searchType", "games"},
      {"searchTerms", search_terms},
      {"searchPage", page},
      {"size", 20},
      {"searchOptions",
       {{"games",
         {{"userId", 0},
          {"platform", ""},
          {"sortCategory", "popular"},
          {"rangeCategory", "main"},
          {"rangeTime", {{"min", 0}, {"max", 0}}},
          {"gameplay",
           {{"perspective", ""},
            {"flow", ""},
            {"genre", ""},
            {"difficulty", ""}}},
          {"rangeYear", {{"max", ""}, {"min", ""}}},
          {"modifier", search_modifier_to_string(search_modifiers)}}},
        {"users", {{"sortCategory", "postcount"}}},
        {"lists", {{"sortCategory", "follows"}}},
        {"filter", ""},
        {"sort", 0},
        {"randomizer", 0}}},
      {"useCache", true}};

  // If api_key is passed add it to the dict
  if (search_info != nullptr && !search_info->api_key.empty()) {
    payload["searchOptions"]["users"]["id"] = search_info->api_key;
  }

  return payload.dump();
}

std::map<std::string, std::string> HTMLRequests::get_title_request_headers() {
  std::map<std::string, std::string> headers = {
      {"User-Agent", get_random_user_agent()}, {"Referer", REFERER_HEADER}};
  return headers;
}

std::map<std::string, std::string>
HTMLRequests::get_title_request_parameters(int game_id) {
  return {{"id", std::to_string(game_id)}};
}

std::optional<std::string>
HTMLRequests::cut_game_title(const std::string &page_source) {
  if (page_source.empty()) {
    return std::nullopt;
  }

  // Use a regex to capture the content between <title> and </title>
  std::regex title_regex("<title>(.*?)</title>");
  std::smatch match;
  if (std::regex_search(page_source, match, title_regex)) {
    std::string title_text = match[1].str();

    // Cut the title text similar to the Python version
    // For example: "How long is A Way Out? | HowLongToBeat" -> "A Way Out"
    if (title_text.length() > 29) {
      std::string cut_title = title_text.substr(12, title_text.length() - 29);
      return cut_title;
    }
  }

  return std::nullopt;
}

// Helper method to perform curl requests
std::optional<std::string> HTMLRequests::perform_curl_request(
    const std::string &url, const std::map<std::string, std::string> &headers,
    const std::string &post_data,
    const std::map<std::string, std::string> &params) {

  CURL *curl = curl_easy_init();
  if (!curl) {
    return std::nullopt;
  }

  std::string response_data;

  // Build URL with parameters if any
  std::string full_url = url;
  if (!params.empty()) {
    full_url += "?";
    for (const auto &param : params) {
      full_url +=
          curl_easy_escape(curl, param.first.c_str(), param.first.length());
      full_url += "=";
      full_url +=
          curl_easy_escape(curl, param.second.c_str(), param.second.length());
      full_url += "&";
    }
    // Remove trailing '&'
    if (full_url.back() == '&') {
      full_url.pop_back();
    }
  }

  // Set URL
  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());

  // Set callback function
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  // Set timeout
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

  // Set headers
  struct curl_slist *curl_headers = nullptr;
  for (const auto &header : headers) {
    std::string header_str = header.first + ": " + header.second;
    curl_headers = curl_slist_append(curl_headers, header_str.c_str());
  }

  // Set post data if any
  if (!post_data.empty()) {
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.length());
    std::string content_length =
        "Content-Length: " + std::to_string(post_data.length());
    curl_headers = curl_slist_append(curl_headers, content_length.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

  // Perform request
  CURLcode res = curl_easy_perform(curl);

  // Check for errors
  long http_code = 0;

  // Get HTTP status code
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  if (res != CURLE_OK) {
    curl_slist_free_all(curl_headers);
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  // Cleanup
  curl_slist_free_all(curl_headers);
  curl_easy_cleanup(curl);

  // Check HTTP status code
  if (http_code == 200) {
    return response_data;
  }

  return std::nullopt;
}

std::optional<std::string>
HTMLRequests::send_web_request(const std::string &game_name,
                               SearchModifiers search_modifiers, int page) {
  auto headers = get_search_request_headers();
  auto search_info_data = send_website_request_getcode(false);

  if (!search_info_data || search_info_data->api_key.empty()) {
    search_info_data = send_website_request_getcode(true);
  }

  // Make the request
  if (search_info_data && !search_info_data->search_url.empty()) {
    SEARCH_URL = std::string(BASE_URL) + search_info_data->search_url;
  }

  // The main method currently is the call to the API search URL
  std::string search_url_with_key = SEARCH_URL + search_info_data->api_key;
  std::string payload =
      get_search_request_data(game_name, search_modifiers, page, nullptr);

  // Make the first request with search_url_with_key
  auto response = perform_curl_request(search_url_with_key, headers, payload);

  if (response) {
    return response;
  }

  // Try to call with the standard url adding the api key to the user
  std::string search_url = SEARCH_URL;
  payload = get_search_request_data(game_name, search_modifiers, page,
                                    search_info_data.get());

  return perform_curl_request(search_url, headers, payload);
}

std::future<std::optional<std::string>> HTMLRequests::send_async_web_request(
    const std::string &game_name, SearchModifiers search_modifiers, int page) {
  return std::async(std::launch::async, [=]() {
    return send_web_request(game_name, search_modifiers, page);
  });
}

std::optional<std::string> HTMLRequests::get_game_title(int game_id) {
  auto params = get_title_request_parameters(game_id);
  auto headers = get_title_request_headers();

  // Request and extract title
  auto response = perform_curl_request(GAME_URL, headers, "", params);

  if (response) {
    return cut_game_title(*response);
  }

  return std::nullopt;
}

std::future<std::optional<std::string>>
HTMLRequests::async_get_game_title(int game_id) {
  return std::async(std::launch::async,
                    [=]() { return get_game_title(game_id); });
}

std::unique_ptr<SearchInformations>
HTMLRequests::send_website_request_getcode(bool parse_all_scripts) {
  auto headers = get_title_request_headers();
  std::string url = std::string(BASE_URL) + "submit";
  auto response = perform_curl_request(url, headers);

  if (response && !response->empty()) {
    std::vector<std::string> matching_scripts;

    std::regex script_regex("<script[^>]+src\\s*=\\s*[\"']([^\"']+)[\"']");
    auto begin =
        std::sregex_iterator(response->begin(), response->end(), script_regex);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
      std::smatch match = *it;
      if (match.size() > 1) {
        std::string src = match[1].str();
        if (parse_all_scripts || src.find("_app-") != std::string::npos) {
          matching_scripts.push_back(src);
        }
      }
    }

    // Loop over found scripts to try and extract the API key
    for (const auto &script_src : matching_scripts) {
      std::string script_url = std::string(BASE_URL);
      script_url.pop_back();
      script_url += script_src;
      auto script_response = perform_curl_request(script_url, headers);
      if (script_response && !script_response->empty()) {
        auto search_info =
            std::make_unique<SearchInformations>(*script_response);
        if (!search_info->api_key.empty()) {
          return search_info;
        }
      }
    }
  }

  return nullptr;
}

std::future<std::unique_ptr<SearchInformations>>
HTMLRequests::async_send_website_request_getcode(bool parse_all_scripts) {
  return std::async(std::launch::async, [=]() {
    return send_website_request_getcode(parse_all_scripts);
  });
}

// Utility function to convert JSON to string
std::string json_to_string(const nlohmann::json &json_obj) {
  return json_obj.dump();
}

} // namespace howlongtobeat
