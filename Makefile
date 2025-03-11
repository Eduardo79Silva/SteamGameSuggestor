CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O2

# Specify the correct include paths (gtest headers are in lib/gtest/googletest/include)
INCLUDE = -Iinclude -Ilib -Ilib/gtest/googletest/include

# Link the libraries correctly; note the change for gtest:
LIBS = -Llib/curl/lib -lcurl -Llib/gtest/build/lib -lgtest -lgtest_main -pthread

SRC_DIR = src
BUILD_DIR = build
BIN = SteamSuggestor.exe

# Source files and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES)) 

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BIN = run_tests

all: $(BIN)

$(BIN): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# Clean build artifacts
clean:
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	@if exist $(BIN) del /F /Q $(BIN)
	@if exist $(TEST_BIN) del /F /Q $(TEST_BIN)

test: $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_BIN) $(TEST_SRC) $(LIBS)

# Makefile to easily run CI scripts

.PHONY: ci-linux ci-windows

# Run Linux CI script
ci-linux:
	@echo "Running Linux CI script..."
	@chmod +x scripts/linux-ci.sh
	@./scripts/linux-ci.sh

# Run Windows CI script (Windows environments)
ci-windows:
	@echo "Running Windows CI script..."
	@scripts\windows-ci-script.bat

# Help target
help:
	@echo "Available targets:"
	@echo "  ci-linux    - Run Linux CI script"
	@echo "  ci-windows  - Run Windows CI script (Windows environments)"
	@echo "  help        - Display this help message"
