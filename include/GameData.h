#pragma once

#include "HttpClient.h"
#include <string>
#include <vector>

class GameData {
public:
  GameData();
  GameData(int id, std::string name, int playtime,
           bool hasCommunityVisibleStats, long int rTimeLastPlayed);
  ~GameData();

  int getId();
  std::string getName();
  int getPlaytime();
  int getGameDuration();
  bool getHasCommunityVisibleStats();
  long int getRTimeLastPlayed();
  std::vector<std::string> getGenres();
  std::vector<std::string> getCategories();
  std::string getFoundName();

  void setId(int id);
  void setName(std::string name);
  void setPlaytime(int playtime);
  void setGameDuration(int gameDuration);
  void setHasCommunityVisibleStats(bool hasCommunityVisibleStats);
  void setRTimeLastPlayed(long int rTimeLastPlayed);
  void setGenres(std::vector<std::string> genres);
  void setCategories(std::vector<std::string> categories);
  void setFoundName(std::string foundName);

  void printGameData();

  void loadGenresAndCategories(HttpClient& client);
  void loadGameDuration();

  bool operator==(const GameData &game) const;

private:
  int m_id;
  std::string m_name;
  int m_playtime;
  std::string m_foundName;
  int m_gameDuration;
  bool m_hasCommunityVisibleStats;
  long int m_rTimeLastPlayed;
  std::vector<std::string> m_genres;
  std::vector<std::string> m_categories;
};
