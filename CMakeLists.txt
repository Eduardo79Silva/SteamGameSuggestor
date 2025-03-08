cmake_minimum_required(VERSION 3.14)
project(SteamSuggestor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard and export compile commands
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect source files
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Define the main executable
add_executable(SteamSuggestor ${SOURCE_FILES})
set_target_properties(SteamSuggestor PROPERTIES 
    OUTPUT_NAME "SteamSuggestor" 
    SUFFIX ".exe"
)

# Target-specific compile options and definitions
target_compile_options(SteamSuggestor PRIVATE -Wall -Wextra -Werror -pedantic -O2)
target_compile_definitions(SteamSuggestor PRIVATE CURL_STATICLIB)

# Include directories for project sources
target_include_directories(SteamSuggestor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Handle libcurl dependency
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    # Fallback to bundled libcurl if system one is not found
    set(CURL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/curl/include)
    set(CURL_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/curl/lib/libcurl.a)
endif()

target_include_directories(SteamSuggestor PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(SteamSuggestor PRIVATE ${CURL_LIBRARIES})

# Windows-specific: Additional libraries for static libcurl
if(WIN32)
    target_link_libraries(SteamSuggestor PRIVATE ws2_32 wldap32 crypt32)
endif()

# Force static linking for standard libraries
set_target_properties(SteamSuggestor PROPERTIES 
    LINK_FLAGS "-static -static-libgcc -static-libstdc++"
)

# Google Test setup
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    set(GTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/gtest)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        if(EXISTS ${GTEST_ROOT}/CMakeLists.txt)
            add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
            set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/googletest/include)
            set(GTEST_LIBRARIES gtest gtest_main)
        else()
            message(FATAL_ERROR "Google Test not found and source not available at ${GTEST_ROOT}")
        endif()
    endif()

    # Create a library target for your production code
    file(GLOB PRODUCTION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    add_library(steamgame ${PRODUCTION_SOURCES})
    target_include_directories(steamgame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    target_link_libraries(steamgame PRIVATE ${CURL_LIBRARIES})

    # Now set up your test target to include test files and link against steamgame
    file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    add_executable(run_tests ${TEST_SOURCES})
    target_include_directories(run_tests PRIVATE ${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    target_link_libraries(run_tests PRIVATE steamgame ${GTEST_LIBRARIES} pthread ${CURL_LIBRARIES})
    enable_testing()
    add_test(NAME UnitTests COMMAND run_tests)
endif()

# Installation rule
install(TARGETS SteamSuggestor DESTINATION bin)

# Custom clean target to remove build artifacts
add_custom_target(cleanall
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Testing
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/SteamSuggestor.exe
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/run_tests
    COMMENT "Cleaning all build artifacts"
)

