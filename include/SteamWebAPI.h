#pragma once

#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

json loadSteamCredentials();

json saveSteamCredentialsLocally(std::string steamApiKey);

std::string askForAPIKey();
