# $(info DDEBUG_MODE is set) - for Makefile debug

# 1. project structure: folder per class for better ownership & readability 
# 2. for explicit and self-documenting code we need to specify the path to the header 
#	relative to the ./src folder

# =========================================================================== #
#                                   TOOLCHAIN                                 #
# =========================================================================== #


NAME := webserv
INCLUDE := -I src
BUILD_DIR := obj
SRC_DIR := src

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17 $(INCLUDE)
DEPFLAGS := -MMD -MP
# -MMD -MP: generate .d dependency files (per .cpp), so that changing a
# header also triggers recompilation, not just changing the .cpp itself.


# =========================================================================== #
#                              BUILD MODE FLAGS                               #
# =========================================================================== #

# make        -> release: NDEBUG defined, assert() compiled out, no logging
# make DBG=1  -> debug: DEBUG_MODE on (LOG_* macros active), assert() active
# make DBG=1 LOG=1 -> debug + LOG_TO_FILE (Logger writes to file, not stdout)

ifdef DBG
	CXXFLAGS += -DDEBUG_MODE
	ifdef LOG
		CXXFLAGS += -DLOG_TO_FILE
	endif
else
	CXXFLAGS += -DNDEBUG
endif

# =========================================================================== #
#                                MAIN BUILD                                   #
# =========================================================================== #
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

# =========================================================================== #
#                                    TESTS                                    #
# =========================================================================== #

TEST_DIR := tests
TEST_BUILD := $(TEST_DIR)/build

# -maxdepth 1: only tests/*.cpp, not subfolders
# Keeps future larger/nested test suites out of `make test` by default.
TEST_SRC := $(shell find $(TEST_DIR) -maxdepth 1 -type f -name "*.cpp")
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp,$(TEST_BUILD)/%,$(TEST_SRC))

DEMO_SRC := $(shell find $(TEST_DIR)/demo -maxdepth 1 -type f -name "*.cpp")
DEMO_BINS := $(patsubst $(TEST_DIR)/demo/%.cpp,$(TEST_BUILD)/%,$(DEMO_SRC))

# Manual list — isolates tests from the rest of src/
# (faster build, no unrelated breakage).
TEST_DEPS := $(SRC_DIR)/utils/logger/logger.cpp
TEST_DEPS += $(SRC_DIR)/io/socket/socket.cpp
TEST_DEPS += $(SRC_DIR)/http/http_parser/http_parser.cpp
TEST_DEPS += $(SRC_DIR)/app/server/server.cpp

# -U after -D cancels previous define
# With FLAGS_MARKER we can call make test and then make test LOG=1
# Without it make can't see a reason to recompile, so it won't create or rewrite the log file
ifdef LOG
	TEST_FLAGS := $(CXXFLAGS) -UNDEBUG $(DEPFLAGS) -DDEBUG_MODE -DLOG_TO_FILE
	FLAGS_TAG := log1
else
	TEST_FLAGS := $(CXXFLAGS) -UNDEBUG $(DEPFLAGS) -DDEBUG_MODE
	FLAGS_TAG := log0
endif

# FLAGS_TAG (log0/log1) picks which flag combo is active for this run
# FLAGS_MARKER is a real file on disk named after the tag, e.g. .last_flags_log1
# It's an empty file, only its timestamp matters to make
#
# test/demo binaries list FLAGS_MARKER as a dependency
# so make rebuilds them if the marker file is newer than the binary
#
# switching log0 <-> log1 always deletes the other marker first
# so make always sees "marker for this tag doesn't exist yet" -> force rebuild

FLAGS_MARKER := $(TEST_BUILD)/.last_flags_$(FLAGS_TAG)

$(TEST_BUILD)/%: $(TEST_DIR)/%.cpp $(TEST_DEPS) $(FLAGS_MARKER)
	@mkdir -p $(TEST_BUILD)
	@$(CXX) $(TEST_FLAGS) $< $(TEST_DEPS) -o $@ -MF $@.d
	@echo "Compiled test: $@"

$(DEMO_BINS): $(TEST_BUILD)/%: $(TEST_DIR)/demo/%.cpp $(TEST_DEPS) $(FLAGS_MARKER)
	@mkdir -p $(TEST_BUILD)
	@$(CXX) $(TEST_FLAGS) $< $(TEST_DEPS) -o $@ -MF $@.d
	@echo "Compiled demo: $@"

$(FLAGS_MARKER):
	@mkdir -p $(TEST_BUILD)
	@rm -f $(TEST_BUILD)/.last_flags_*
	@touch $(FLAGS_MARKER)

# shell script runs each test binary one by one, keeps two string lists: passed/failed names
# total/npassed/nfailed are just counters built with $$(( )) and wc -w
test: $(TEST_BINS)
	@echo "\nRunning tests..."
	@ failed=""; npassed=0; total=0; \
	for bin in $(TEST_BINS); do \
		name=$$(basename $$bin); \
		echo "\n\--- $$bin ---"; \
		if ./$$bin;  then \
			npassed=$$((npassed + 1)); \
		else \
			failed="$$failed $$name"; \
		fi; \
		total=$$((total + 1));\
	done; \
	nfailed=$$(echo $$failed | wc -w | tr -d ' '); \
	printf "\n$(BOLD)==================== SUMMARY ====================$(RESET)\n"; \
	printf "Test files: $$total total, $(GREEN)$$npassed passed$(RESET), $(RED)$$nfailed failed$(RESET)\n"; \
	if [ -n "$$failed" ]; then printf "$(RED)FAILED:$(RESET)$$failed\n"; fi; \
	if [ -n "$$failed" ]; then exit 1; fi

demo: $(DEMO_BINS)
	@echo "\nRunning demos..."
	@for bin in $(DEMO_BINS); do \
		echo "\n--- $$bin ---"; \
		./$$bin; \
	done

test_clean:
	@rm -rf $(TEST_BUILD)
	@echo "Tests obj & binary cleaned up."

# =========================================================================== #
#                                  PHONY / DEPS                               #
# =========================================================================== #

.PHONY: all clean fclean re test test_clean demo

-include $(DEPS)
-include $(addsuffix .d, $(TEST_BINS)) $(addsuffix .d, $(DEMO_BINS))

# =========================================================================== #
#                                  ANSI COLORS                                #
# =========================================================================== #

GREEN  := \033[0;32m
RED    := \033[0;31m
BOLD   := \033[1m
RESET  := \033[0m