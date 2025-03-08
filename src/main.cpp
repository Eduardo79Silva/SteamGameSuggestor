#include "GamesManager.h"
#include "HttpClient.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <thread>

using json = nlohmann::json;

int main() {
  // Open the config file
  std::ifstream configFile("config.json");
  if (!configFile) {
    configFile.close();
    configFile.clear();
    configFile.open("../config.json");
    if (!configFile) {
      std::cerr << "Error: Could not open config file!" << std::endl;
      return 1;
    }
  }

  // Parse the JSON config file
  json config;
  try {
    configFile >> config;
  } catch (const json::parse_error &e) {
    std::cerr << "Error parsing config file: " << e.what() << std::endl;
    return 1;
  }

  // Access configuration parameters
  std::string apiKey = config.value("steamApiKey", "");

  std::string url =
      "https://api.steampowered.com/IPlayerService/GetOwnedGames/v0001/"
      "?key=" +
      apiKey +
      "&steamid=76561198190657579&format=json&include_appinfo=true&include_"
      "played_free_games=true";

  std::cout << "API URL: " << url << std::endl;

  HttpClient client = HttpClient();

  auto ownedGames = client.callAPI(url);

  try {
    GamesManager gamesManager(ownedGames);
    // In your test or main function:
    // make a call to loadGamesGenresAndCategories run in the background
    std::thread t(&GamesManager::loadGamesGenresAndCategories, &gamesManager);
    std::thread t2(&GamesManager::loadGamesDuration, &gamesManager);

    // Ask the user for a game namesp
    std::string gameName;

    // Search for the gameName
    t.join();
    t2.join();
    gamesManager.printGames();

  } catch (const std::exception &e) {
    // Catch standard exceptions
    std::cerr << "Standard exception: " << e.what() << std::endl;
  } catch (_exception e) {
    // Catch your custom exceptions
    std::cerr << "Error: " << e.name << std::endl;
  } catch (...) {
    // Catch any other exceptions
    std::cerr << "Unknown error occurred" << std::endl;
  }

  return 0;
}
