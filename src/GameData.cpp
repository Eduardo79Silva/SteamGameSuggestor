#include "GameData.h"
#include "HLTB.h"
#include "HttpClient.h"
#include "utils.h"
#include <iostream>
#include <ostream>
#include <string>

GameData::GameData() {
  m_id = 0;
  m_name = "";
  m_playtime = 0;
  m_hasCommunityVisibleStats = false;
  m_rTimeLastPlayed = 0;
}

GameData::GameData(int id, std::string name, int playtime,
                   bool hasCommunityVisibleStats, long int rTimeLastPlayed) {
  this->m_id = id;
  this->m_name = name;
  this->m_playtime = playtime;
  this->m_hasCommunityVisibleStats = hasCommunityVisibleStats;
  this->m_rTimeLastPlayed = rTimeLastPlayed;
}

GameData::~GameData() {}

int GameData::getId() { return m_id; }

std::string GameData::getName() { return m_name; }

int GameData::getPlaytime() { return m_playtime; }

int GameData::getGameDuration() { return m_gameDuration; }

bool GameData::getHasCommunityVisibleStats() {
  return m_hasCommunityVisibleStats;
}

long int GameData::getRTimeLastPlayed() { return m_rTimeLastPlayed; }

std::vector<std::string> GameData::getGenres() { return m_genres; }

std::vector<std::string> GameData::getCategories() { return m_categories; }

void GameData::setId(int id) { this->m_id = id; }

void GameData::setName(std::string name) { this->m_name = name; }

void GameData::setPlaytime(int playtime) { this->m_playtime = playtime; }

void GameData::setGameDuration(int gameDuration) {
  this->m_gameDuration = gameDuration;
}

void GameData::setHasCommunityVisibleStats(bool hasCommunityVisibleStats) {
  this->m_hasCommunityVisibleStats = hasCommunityVisibleStats;
}

void GameData::setRTimeLastPlayed(long int rTimeLastPlayed) {
  this->m_rTimeLastPlayed = rTimeLastPlayed;
}

void GameData::setGenres(std::vector<std::string> genres) {
  this->m_genres = genres;
}

void GameData::setCategories(std::vector<std::string> categories) {
  this->m_categories = categories;
}

std::string GameData::getFoundName() { return m_foundName; }

void GameData::setFoundName(std::string foundName) {
  this->m_foundName = foundName;
}

void GameData::printGameData() {
  // print game data in one line
  std::cout << "ID: " << m_id << " Name: " << m_name
            << " Playtime: " << m_playtime
            << " Has Community Visible Stats: " << m_hasCommunityVisibleStats
            << " RTime Last Played: "
            << unixTimeToHumanReadable(m_rTimeLastPlayed)
            << " Genres: " << m_genres.size()
            << " Categories: " << m_categories.size()
            << " Game Duration: " << m_gameDuration
            << " Found Name: " << m_foundName << std::endl;
}

void GameData::loadGenresAndCategories(HttpClient &client) {
  auto gameInfo =
      client.callAPI("https://store.steampowered.com/api/appdetails?appids=" +
                     std::to_string(getId()));

  if (gameInfo.size() > 0 && gameInfo[std::to_string(getId())]["success"]) {
    auto genres = gameInfo[std::to_string(getId())]["data"]["genres"];
    for (auto &genre : genres) {
      m_genres.push_back(genre["description"]);
    }

    auto categories = gameInfo[std::to_string(getId())]["data"]["categories"];
    for (auto &category : categories) {
      m_categories.push_back(category["description"]);
    }
  }
}

void GameData::loadGameDuration() {
  // Asynchronous request example
  if (m_name.find("Wallpaper Engine") != std::string::npos or
      m_name.find("Test") != std::string::npos or
      m_name.find("Demo") != std::string::npos) {
    m_gameDuration = 0;
    return;
  }
  auto result = howlongtobeat::HTMLRequests::send_web_request(m_name);

  // Get the result when ready
  if (result) {
    try {
      auto json_result = nlohmann::json::parse(*result);

      // Print the first few results
      auto &data = json_result["data"];
      if (data[0].contains("comp_main") && !data[0]["comp_main"].is_null()) {
        m_gameDuration = data[0]["comp_main"].get<int>() / 3600;
        m_foundName = data[0]["game_name"].get<std::string>();
      }
      else {
        m_gameDuration = 0;
      }
    } catch (const nlohmann::json::exception &e) {
      std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Async search failed" << std::endl;
  }
}

bool GameData::operator==(const GameData &game) const {
  return m_id == game.m_id;
}
