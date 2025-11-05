.PHONY: build test clean help all

BINARY_NAME=thingino-cloner
BUILD_DIR=build
BIN_DIR=bin

# Build the project using CMake
build:
	@echo "Building $(BINARY_NAME)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make
	@echo "Build complete: $(BUILD_DIR)/$(BINARY_NAME)"

# Build and copy to bin directory
all: build
	@mkdir -p $(BIN_DIR)
	@cp $(BUILD_DIR)/$(BINARY_NAME) $(BIN_DIR)/
	@echo "Binary copied to $(BIN_DIR)/$(BINARY_NAME)"

# Run tests
test: build
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@rm -f $(BINARY_NAME)

# Rebuild from scratch
rebuild: clean build

# Help
help:
	@echo "Available targets:"
	@echo "  build       - Build the project using CMake"
	@echo "  all         - Build and copy binary to bin/"
	@echo "  test        - Run tests"
	@echo "  clean       - Clean build artifacts"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  help        - Show this help message"