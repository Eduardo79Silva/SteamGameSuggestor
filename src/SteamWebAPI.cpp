#include <SteamWebAPI.h>
#include <fstream>
#include <iostream>

json loadSteamCredentials() {

  // Open the config file
  std::ifstream configFile("config.json");
  if (!configFile) {
    configFile.close();
    configFile.clear();
    configFile.open("../config.json");
    if (!configFile) {
      std::cerr << "Error: Could not open config file!" << std::endl;
      std::string key = askForAPIKey();
      return saveSteamCredentialsLocally(key);
    }
  }

  // Parse the JSON config file
  json config;
  try {
    configFile >> config;
  } catch (const json::parse_error &e) {
    std::cerr << "Error parsing config file: " << e.what() << std::endl;
    std::string key = askForAPIKey();
    return saveSteamCredentialsLocally(key);
  }

  return config;
}

json saveSteamCredentialsLocally(std::string steamApiKey) {
  std::ofstream config("config.json");
  if (config.is_open()) {
    json configDefault = json().value("steamApiKey", steamApiKey);

    config << configDefault;

    return configDefault;
  }
  return json();
}

std::string askForAPIKey() {
  std::string key;

  std::cout << "Please insert the API Key that you can find here: "
               "https://steamcommunity.com/dev/apikey";
  std::cout << "When you are asked for a domain simply insert: localhost.";

  std::cin >> key;

  return key;
}
