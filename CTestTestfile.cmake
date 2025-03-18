# CMake generated Testfile for 
# Source directory: D:/Minhas coisas/Hustle/SteamGameSuggestor
# Build directory: D:/Minhas coisas/Hustle/SteamGameSuggestor
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests "D:/Minhas coisas/Hustle/SteamGameSuggestor/run_tests.exe")
set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "D:/Minhas coisas/Hustle/SteamGameSuggestor/CMakeLists.txt;83;add_test;D:/Minhas coisas/Hustle/SteamGameSuggestor/CMakeLists.txt;0;")
subdirs("gtest")
