# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -I./src
LIBS = -lssl -lcrypto

# Directories
SRC_DIR = src
BIN_DIR = bin
DATA_DIR = data

# Source and object files
SOURCES = $(SRC_DIR)/sign.cpp $(SRC_DIR)/verify.cpp $(SRC_DIR)/util.cpp
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SOURCES))
EXECS = $(BIN_DIR)/sign $(BIN_DIR)/verify

# Default target
all: $(BIN_DIR) $(EXECS)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir $(BIN_DIR)

# Compile source to object
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executables
$(BIN_DIR)/sign: $(BIN_DIR)/sign.o $(BIN_DIR)/util.o
	$(CXX) $^ $(LIBS) -o $@

$(BIN_DIR)/verify: $(BIN_DIR)/verify.o $(BIN_DIR)/util.o
	$(CXX) $^ $(LIBS) -o $@

# Detect OS and set clean command
ifeq ($(OS),Windows_NT)
    CLEAN_CMD = del /Q
    EXEC_EXT = .exe
    MKDIR_CMD = mkdir
else
    CLEAN_CMD = rm -f
    EXEC_EXT =
    MKDIR_CMD = mkdir -p
endif

# Clean up
clean:
	$(CLEAN_CMD) $(BIN_DIR)\*.o $(BIN_DIR)\sign$(EXEC_EXT) $(BIN_DIR)\verify$(EXEC_EXT)

# Phony targets
.PHONY: all clean
