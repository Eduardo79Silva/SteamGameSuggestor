// howlongtobeat.h
#ifndef HOWLONGTOBEAT_H
#define HOWLONGTOBEAT_H

#include <nlohmann/json.hpp>
#include <cstddef>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace howlongtobeat {

// Enumeration for search modifiers
enum class SearchModifiers {
  NONE,
  ISOLATE_DLC,   // Shows only DLC in the search result
  ISOLATE_MODS,  // Shows only MODs in the search result
  ISOLATE_HACKS, // Shows only Hacks in the search result
  HIDE_DLC       // Hides DLCs/MODs in the search result
};

// Class to hold search information extracted from website
class SearchInformations {
public:
  std::string search_url;
  std::string api_key;

  explicit SearchInformations(const std::string &script_content);

private:
  std::string extract_api_from_script(const std::string &script_content);
  std::string extract_search_url_script(const std::string &script_content);
};

// Class to handle HTTP requests
class HTMLRequests {
public:
  static constexpr const char *BASE_URL = "https://www.howlongtobeat.com/";
  static constexpr const char *REFERER_HEADER = BASE_URL;
  static constexpr const char *GAME_URL = "https://www.howlongtobeat.com/game";
  static std::string SEARCH_URL;

  static std::map<std::string, std::string> get_search_request_headers();
  static std::string
  get_search_request_data(const std::string &game_name,
                          SearchModifiers search_modifiers, int page,
                          const SearchInformations *search_info);

  static std::optional<std::string>
  send_web_request(const std::string &game_name,
                   SearchModifiers search_modifiers = SearchModifiers::NONE,
                   int page = 1);

  static std::future<std::optional<std::string>> send_async_web_request(
      const std::string &game_name,
      SearchModifiers search_modifiers = SearchModifiers::NONE, int page = 1);

  static std::optional<std::string> get_game_title(int game_id);
  static std::future<std::optional<std::string>>
  async_get_game_title(int game_id);

  static std::unique_ptr<SearchInformations>
  send_website_request_getcode(bool parse_all_scripts);
  static std::future<std::unique_ptr<SearchInformations>>
  async_send_website_request_getcode(bool parse_all_scripts);

private:
  static std::map<std::string, std::string> get_title_request_headers();
  static std::map<std::string, std::string>
  get_title_request_parameters(int game_id);
  static std::optional<std::string>
  cut_game_title(const std::string &page_source);
  static std::string get_random_user_agent();

  // Helper method for curl requests
  static size_t write_callback(char *ptr, size_t size, size_t nmemb,
                               std::string *data);
  static std::optional<std::string>
  perform_curl_request(const std::string &url,
                       const std::map<std::string, std::string> &headers,
                       const std::string &post_data = "",
                       const std::map<std::string, std::string> &params = {});
};

// Utility functions
std::string search_modifier_to_string(SearchModifiers modifier);
std::string json_to_string(const nlohmann::json &json_obj);

} // namespace howlongtobeat

#endif // HOWLONGTOBEAT_H
