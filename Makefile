
# 1. project structure: folder per class for better ownership & readability 
# 2. for explicit and self-documenting code we need to specify the path to the header 
#	relative to the ./src folder



NAME := webserv
INCLUDE := -I src
BUILD_DIR := obj
SRC_DIR := src

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17 $(INCLUDE)

# run shell funcion to find only files in src with name *.cpp and return output without main.cpp
SRC :=  $(shell find src -type f -name "*.cpp" ! -name "main.cpp")

SRC_MAIN := $(SRC_DIR)/main.cpp
SRC_RUN_APP := $(SRC_DIR)/run_app.cpp

SRC_FULL := $(SRC) $(SRC_MAIN)

OBJ := $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FULL))

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(NAME) successfully built."

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled $< successfully."

clean:
	@rm -fr $(BUILD_DIR)
	@echo "Object files cleaned."

fclean: clean
	@rm -f $(NAME)
	@echo "Project cleaning is finished."

re: fclean all

TEST_DIR := tests
TEST_BUILD := $(TEST_DIR)/build
TEST_SRC := $(shell find $(TEST_DIR) -maxdepth 1 -type f -name "*.cpp")
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp, $(TEST_BUILD)/%, $(TEST_SRC))

$(TEST_BUILD)/%: $(TEST_DIR)/%.cpp
	@mkdir -p $(TEST_BUILD)
	@$(CXX) $(CXXFLAGS) $< -o $@
	@echo "Compiled test: $@"

test: $(TEST_BINS)
	@echo "\nRunning tests...\n"
	@for bin in $(TEST_BINS); do \
		echo "--- $$bin ---"; \
		./$$bin; \
	done

tests_clean:
	@rm -rf $(TEST_BUILD)
	@echo "Tests obj & binary cleaned up."

.PHONY: all clean fclean re test