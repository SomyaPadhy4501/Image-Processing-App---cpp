CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -isystem lib
LDFLAGS =

BUILD_DIR = build
GTEST_DIR = $(BUILD_DIR)/googletest/googletest
GTEST_INC = $(GTEST_DIR)/include
GTEST_SRC = $(GTEST_DIR)/src

LIB_SOURCES = src/model/ImageModel.cpp

.PHONY: all clean test app

all: app

app: $(BUILD_DIR)/image_processor

$(BUILD_DIR)/image_processor: $(LIB_SOURCES) src/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o "$@" $(LIB_SOURCES) src/main.cpp

$(BUILD_DIR)/googletest:
	@echo "Downloading Google Test..."
	@mkdir -p "$(BUILD_DIR)"
	@cd "$(BUILD_DIR)" && git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git 2>/dev/null || true

$(BUILD_DIR)/libgtest.a: $(BUILD_DIR)/googletest
	cd "$(GTEST_DIR)" && $(CXX) -std=c++17 -O2 -isystem "include" -I"." -c src/gtest-all.cc -o gtest-all.o
	mv "$(GTEST_DIR)/gtest-all.o" "$(BUILD_DIR)/gtest-all.o"
	ar rcs "$@" "$(BUILD_DIR)/gtest-all.o"

TEST_SOURCES = $(wildcard tests/*.cpp)

$(BUILD_DIR)/run_tests: $(LIB_SOURCES) $(TEST_SOURCES) $(BUILD_DIR)/libgtest.a | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -isystem "$(GTEST_INC)" -I"$(GTEST_DIR)" -o "$@" \
		$(LIB_SOURCES) $(TEST_SOURCES) \
		"$(GTEST_SRC)/gtest_main.cc" \
		"$(BUILD_DIR)/libgtest.a" \
		-lpthread

test: $(BUILD_DIR)/run_tests
	@echo "Running tests..."
	@"./$(BUILD_DIR)/run_tests"

$(BUILD_DIR):
	mkdir -p "$(BUILD_DIR)"

clean:
	rm -rf "$(BUILD_DIR)"
