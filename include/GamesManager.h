#pragma once

#include <nlohmann/json.hpp>
#include "GameData.h"
#include <vector>

class GamesManager {
public:
  GamesManager();
  GamesManager(nlohmann::json gameResponse);
  ~GamesManager();

  std::vector<GameData> getGames();
  std::vector<GameData> getGamesWithGenre(std::string genre);
  std::vector<GameData> getGamesWithCategory(std::string category);
  std::vector<GameData> getPlayedGames();
  std::vector<GameData> getUnplayedGames();

  void addGame(GameData game);

  void printGames();

  void printGamesWithGenre(std::string genre);
  void printGamesWithCategory(std::string category);
  void setGames(std::vector<GameData> games);

  void loadGamesGenresAndCategories();
  void loadGamesDuration();

private:
  std::vector<GameData> m_games;
};
