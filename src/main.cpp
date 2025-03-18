#include "GamesManager.h"
#include "HttpClient.h"
#include <SteamWebAPI.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <thread>

int main() {

  json config = loadSteamCredentials();

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
  } catch (...) {
    // Catch any other exceptions
    std::cerr << "Unknown error occurred" << std::endl;
  }

  return 0;
}
