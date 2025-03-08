#include "GamesManager.h"
#include <gtest/gtest.h>

// Test fixture for GamesManager
class GamesManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    game1 = GameData(123, "Game1", 0, true, 0);
    game2 = GameData(456, "Game2", 1, true, 0);

    game1.setGenres({"Genre1"});
    game1.setCategories({"Category1"});
    game2.setGenres({"Genre2"});
    game2.setCategories({"Category2"});

    gamesManager.addGame(game1);
    gamesManager.addGame(game2);
  }

  GamesManager gamesManager;
  GameData game1;
  GameData game2;
};

TEST_F(GamesManagerTest, InitiallyNoGames) {
  GamesManager emptyManager;
  EXPECT_EQ(0, emptyManager.getGames().size());
}

TEST_F(GamesManagerTest, AddGameIncreasesCount) {
  GameData newGame(789, "Game3", 2, true, 0);
  gamesManager.addGame(newGame);
  EXPECT_EQ(3, gamesManager.getGames().size());
}

TEST_F(GamesManagerTest, RetrieveGamesWithSpecificGenre) {
  auto games = gamesManager.getGamesWithGenre("Genre1");
  ASSERT_EQ(1, games.size());
  EXPECT_EQ("Game1", games[0].getName());
}

TEST_F(GamesManagerTest, RetrieveGamesWithSpecificCategory) {
  auto games = gamesManager.getGamesWithCategory("Category2");
  ASSERT_EQ(1, games.size());
  EXPECT_EQ("Game2", games[0].getName());
}

TEST_F(GamesManagerTest, RetrievePlayedGames) {
  auto games = gamesManager.getPlayedGames();
  ASSERT_EQ(1, games.size());
  EXPECT_EQ("Game2", games[0].getName());
}

TEST_F(GamesManagerTest, RetrieveUnplayedGames) {
  auto games = gamesManager.getUnplayedGames();
  ASSERT_EQ(1, games.size());
  EXPECT_EQ("Game1", games[0].getName());
}

TEST_F(GamesManagerTest, SetGamesReplacesExistingGames) {
  GameData newGame(789, "Game3", 2, true, 0);
  gamesManager.setGames({newGame});
  auto games = gamesManager.getGames();
  ASSERT_EQ(1, games.size());
  EXPECT_EQ("Game3", games[0].getName());
}

TEST_F(GamesManagerTest, AddDuplicateGameDoesNotIncreaseCount) {
  gamesManager.addGame(game1);
  EXPECT_EQ(2, gamesManager.getGames().size());
}

TEST_F(GamesManagerTest, HandleGameWithNoGenresOrCategories) {
  GameData newGame(789, "Game3", 2, true, 0);
  gamesManager.addGame(newGame);
  auto games = gamesManager.getGames();
  ASSERT_EQ(3, games.size());
  EXPECT_TRUE(games[2].getGenres().empty());
  EXPECT_TRUE(games[2].getCategories().empty());
}

TEST_F(GamesManagerTest, HandleGameWithHighPlaytime) {
  GameData newGame(789, "Game3", 1000000, true, 0);
  gamesManager.addGame(newGame);
  auto games = gamesManager.getPlayedGames();
  ASSERT_EQ(2, games.size());
  EXPECT_EQ(1000000, games[1].getPlaytime());
}

