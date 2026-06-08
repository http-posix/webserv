
# 1. project structure: folder per class for better ownership & readability 
# 2. for explicit and self-documenting code we need to specify the path to the header 
#	relative to the ./src folder



NAME := webserv
INCLUDE := -I src
BUILD_DIR := obj
SRC_DIR := src

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17 $(INCLUDE)
DEPFLAGS := -MMD -MP

# $(info DDEBUG_MODE is set) - for Makefile debug

ifdef DBG
	CXXFLAGS += -DDEBUG_MODE
	ifdef LOG
		CXXFLAGS += -DLOG_TO_FILE
	endif
endif

# run shell funcion to find only files in src with name *.cpp and return output without main.cpp
SRC :=  $(shell find src -type f -name "*.cpp" ! -name "main.cpp")

SRC_MAIN := $(SRC_DIR)/main.cpp

SRC_FULL := $(SRC) $(SRC_MAIN)

OBJ := $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FULL))
DEPS := $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(NAME) successfully built."

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@
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
TEST_DEPS := $(SRC_DIR)/logger/logger.cpp
TEST_DEPS += $(SRC_DIR)/socket/socket.cpp
TEST_DEPS += $(SRC_DIR)/http_parser/http_parser.cpp
TEST_FLAGS = $(CXXFLAGS) $(DEPFLAGS) -DDEBUG_MODE -DLOG_TO_FILE

$(TEST_BUILD)/%: $(TEST_DIR)/%.cpp $(TEST_DEPS)
	@mkdir -p $(TEST_BUILD)
	@$(CXX) $(TEST_FLAGS) $< $(TEST_DEPS) -o $@ -MF $@.d
	@echo "Compiled test: $@"

test: $(TEST_BINS)
	@echo "\nRunning tests..."
	@set -e; \
	for bin in $(TEST_BINS); do \
		echo "\n\--- $$bin ---"; \
		./$$bin; \
	done

test_clean:
	@rm -rf $(TEST_BUILD)
	@echo "Tests obj & binary cleaned up."

.PHONY: all clean fclean re test test_clean

-include $(DEPS)
-include $(addsuffix .d, $(TEST_BINS))
