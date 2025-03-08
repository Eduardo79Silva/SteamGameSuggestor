#include "GamesManager.h"
#include "utils.h"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <fstream>
#include <future>
#include <iostream>

GamesManager::GamesManager() {}

GamesManager::GamesManager(nlohmann::json gameResponse) {
  for (auto &game : gameResponse["response"]["games"]) {
    std::string cleanName = removeSpecialCharacters(game["name"]);
    GameData gameData(game["appid"], cleanName, game["playtime_forever"],
                      game["has_community_visible_stats"] != 0 ? true : false,
                      game["rtime_last_played"]);
    m_games.push_back(gameData);
  }
}

GamesManager::~GamesManager() {}

void GamesManager::addGame(GameData game) {
  if (std::find(m_games.begin(), m_games.end(), game) == m_games.end()) {
    m_games.push_back(game);
  }
}

void GamesManager::printGames() {
  for (auto &game : m_games) {
    game.printGameData();
  }
}

std::vector<GameData> GamesManager::getGames() { return m_games; }

std::vector<GameData> GamesManager::getGamesWithGenre(std::string genre) {
  std::vector<GameData> gamesWithGenre;

  for (auto &game : m_games) {
    auto genreList = game.getGenres(); // Store it in a variable
    if (std::find(genreList.begin(), genreList.end(), genre) !=
        genreList.end()) {
      gamesWithGenre.push_back(game);
    }
  }
  return gamesWithGenre;
}

std::vector<GameData> GamesManager::getGamesWithCategory(std::string category) {
  std::vector<GameData> gamesWithCategory;
  for (auto &game : m_games) {
    auto categoryList = game.getCategories();
    if (std::find(categoryList.begin(), categoryList.end(), category) !=
        categoryList.end()) {
      gamesWithCategory.push_back(game);
    }
  }
  return gamesWithCategory;
}

std::vector<GameData> GamesManager::getPlayedGames() {
  std::vector<GameData> playedGames;
  for (auto &game : m_games) {
    if (game.getPlaytime() > 0) {
      playedGames.push_back(game);
    }
  }
  return playedGames;
}

std::vector<GameData> GamesManager::getUnplayedGames() {
  std::vector<GameData> unplayedGames;
  for (auto &game : m_games) {
    if (game.getPlaytime() == 0) {
      unplayedGames.push_back(game);
    }
  }
  return unplayedGames;
}

void GamesManager::setGames(std::vector<GameData> games) {
  this->m_games = games;
}

void GamesManager::printGamesWithGenre(std::string genre) {
  for (auto &game : m_games) {
    if (std::find(game.getGenres().begin(), game.getGenres().end(), genre) !=
        game.getGenres().end()) {
      game.printGameData();
    }
  }
}

void GamesManager::printGamesWithCategory(std::string category) {
  for (auto &game : m_games) {
    if (std::find(game.getCategories().begin(), game.getCategories().end(),
                  category) != game.getCategories().end()) {
      game.printGameData();
    }
  }
}

void GamesManager::loadGamesGenresAndCategories() {
  // Create a cache directory if it doesn't exist
  std::filesystem::create_directories("cache");
  
  std::vector<std::future<void>> futures;
  std::atomic<int> cacheHits(0);
  std::atomic<int> cacheMisses(0);
  
  // Process games with limited concurrency
  const size_t MAX_CONCURRENT = 10;
  for (size_t i = 0; i < m_games.size(); ++i) {
    futures.push_back(std::async(std::launch::async, [this, i, &cacheHits, &cacheMisses]() {
      auto& game = m_games[i];
      std::string cacheFile = "cache/info_" + std::to_string(game.getId()) + ".cache";
      
      // Check if we have cached data
      if (std::filesystem::exists(cacheFile)) {
        std::ifstream cache(cacheFile);
        if (cache.good()) {
          try {
            // Read JSON from cache file
            nlohmann::json gameData;
            cache >> gameData;
            
            // Apply cached data to game object
            if (gameData.contains("genres") && gameData.contains("categories")) {
              std::vector<std::string> genres = gameData["genres"].get<std::vector<std::string>>();
              std::vector<std::string> categories = gameData["categories"].get<std::vector<std::string>>();
              
              game.setGenres(genres);
              game.setCategories(categories);
              
              cacheHits++;
              return;
            }
          } catch (const std::exception& e) {
            std::cerr << "Error reading cache for game " << game.getId() 
                      << ": " << e.what() << std::endl;
            // Continue to load from API if cache read fails
          }
        }
      }
      
      // No cache hit or cache read failed, load from API
      cacheMisses++;
      HttpClient client;
      game.loadGenresAndCategories(client);
      
      // Cache the result
      try {
        nlohmann::json gameData;
        gameData["genres"] = game.getGenres();
        gameData["categories"] = game.getCategories();
        
        std::ofstream cache(cacheFile);
        cache << gameData.dump(2); // Pretty-print with 2-space indentation
      } catch (const std::exception& e) {
        std::cerr << "Error writing cache for game " << game.getId() 
                  << ": " << e.what() << std::endl;
      }
    }));
  }
  
  // Process futures in batches to limit concurrency
  for (size_t i = 0; i < futures.size(); i += MAX_CONCURRENT) {
    size_t end = std::min(i + MAX_CONCURRENT, futures.size());
    
    for (size_t j = i; j < end; ++j) {
      futures[j].wait();
    }
    
    // Progress indicator
    std::cout << "Genres/Categories loading: " << std::min(end, futures.size()) 
              << "/" << futures.size() << " games processed ("
              << (end * 100 / futures.size()) << "%)" << std::endl;
  }
  
  std::cout << "Info loading complete. Cache hits: " << cacheHits 
            << ", Cache misses: " << cacheMisses << std::endl;
}

void GamesManager::loadGamesDuration() {
  // Create a cache directory if it doesn't exist
  std::filesystem::create_directories("cache");

  std::vector<std::future<void>> futures;
  std::atomic<int> cacheHits(0);
  std::atomic<int> cacheMisses(0);

  // Process only games that need duration data
  for (size_t i = 0; i < m_games.size(); ++i) {
    futures.push_back(
        std::async(std::launch::async, [this, i, &cacheHits, &cacheMisses]() {
          auto &game = m_games[i];
          std::string cacheFile =
              "cache/duration_" + std::to_string(game.getId()) + ".cache";

          // Check if we have cached data
          if (std::filesystem::exists(cacheFile)) {
            std::ifstream cache(cacheFile);
            int duration;
            if (cache >> duration) {
              game.setGameDuration(duration);
              cacheHits++;
              return;
            }
          }

          // No cache hit, load from API
          cacheMisses++;
          game.loadGameDuration();

          // Cache the result
          std::ofstream cache(cacheFile);
          cache << game.getGameDuration();
        }));
  }

  // Limit concurrency to avoid overwhelming the API
  const size_t MAX_CONCURRENT = 10;
  for (size_t i = 0; i < futures.size(); i += MAX_CONCURRENT) {
    size_t end = std::min(i + MAX_CONCURRENT, futures.size());

    for (size_t j = i; j < end; ++j) {
      futures[j].wait();
    }
  }

  std::cout << "Duration loading complete. Cache hits: " << cacheHits
            << ", Cache misses: " << cacheMisses << std::endl;
}
