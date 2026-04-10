CC := gcc
CFLAGS := -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -pedantic -std=gnu11
CPPFLAGS :=
LIBS := -lcriterion -lm

BUILD_DIR := build
BIN_DIR := bin
TEST_INPUT_DIR := tests.in
TEST_OUTPUT_DIR := tests.out
TEST_RESULTS := test_results.json

HW6_SRC := hw6.c
UNIT_TEST_SRC := unit_tests.c
STUDENT_TEST_SRC := student_tests.c

HW6_OBJ := $(BUILD_DIR)/hw6.o
UNIT_TEST_OBJ := $(BUILD_DIR)/unit_tests.o
STUDENT_TEST_OBJ := $(BUILD_DIR)/student_tests.o

EXEC := hw6
TEST := unit_tests
STUDENT_TEST := student_tests

all: setup $(BIN_DIR)/$(EXEC) $(BIN_DIR)/$(TEST)

debug: CFLAGS += -g -DDEBUG
debug: all

setup:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(TEST_INPUT_DIR)
	@mkdir -p $(TEST_OUTPUT_DIR)

$(BIN_DIR)/$(EXEC): $(HW6_OBJ) | setup
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/$(TEST): $(UNIT_TEST_OBJ) | setup
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BIN_DIR)/$(STUDENT_TEST): $(STUDENT_TEST_OBJ) | setup
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BUILD_DIR)/%.o: %.c | setup
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: all
	@rm -fr $(TEST_INPUT_DIR) $(TEST_OUTPUT_DIR)
	@mkdir -p $(TEST_INPUT_DIR)
	@mkdir -p $(TEST_OUTPUT_DIR)
	@$(BIN_DIR)/$(TEST) --full-stats --verbose --json=$(TEST_RESULTS) -j1

clean:
	rm -fr $(BUILD_DIR) $(BIN_DIR) $(TEST_INPUT_DIR) $(TEST_OUTPUT_DIR) *.out $(TEST_RESULTS)

.PHONY: all clean debug setup test
