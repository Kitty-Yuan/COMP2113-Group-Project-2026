# Makefile for KnightMazeRPG
# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS := -lncurses

# Directories
SRC_DIR := .
UI_DIR := ui
SAVE_DIR := user_save_system
BUILD_DIR := build

# Source files
SOURCES := comp2113.cpp \
           ui/ui_ux.cpp \
           user_save_system/user_save_system.cpp

# Object files (placed in build directory)
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))

# Executable
EXECUTABLE := $(BUILD_DIR)/comp2113

# Phony targets
.PHONY: all clean help

# Default target - clean before building
all: clean $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(OBJECTS)
	@echo "Linking $@..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete! Run with: ./$(EXECUTABLE)"

# Compile source files to object files
$(BUILD_DIR)/comp2113.o: comp2113.cpp | $(BUILD_DIR)
	@echo "Compiling comp2113.cpp..."
	$(CXX) $(CXXFLAGS) -I$(UI_DIR) -I$(SAVE_DIR) -c $< -o $@

$(BUILD_DIR)/ui_ux.o: ui/ui_ux.cpp ui/ui_ux.h | $(BUILD_DIR)
	@echo "Compiling ui/ui_ux.cpp..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/user_save_system.o: user_save_system/user_save_system.cpp user_save_system/user_save_system.h | $(BUILD_DIR)
	@echo "Compiling user_save_system/user_save_system.cpp..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/comp2113
	@echo "Clean complete!"

# Help
help:
	@echo "KnightMazeRPG Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all    - Build the project (default)"
	@echo "  clean  - Remove build artifacts"
	@echo "  help   - Display this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make         # Build the project"
	@echo "  make clean   # Clean build files"
	@echo "  ./build/comp2113  # Run the program"
