CXX       := c++
CXXFLAGS  := --std=c++20 -Wall -Wextra
SRC_DIR   := src
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/unduck

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC_DIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
