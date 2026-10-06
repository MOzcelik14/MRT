CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
DEBUG_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O0 -g -fsanitize=address,undefined
LDFLAGS ?= -lm

SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin

TARGET = mrt
TEST_TARGET = $(BIN_DIR)/mrt_test

SRCS = $(SRC_DIR)/token.c \
       $(SRC_DIR)/error.c \
       $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/ast.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/value.c \
       $(SRC_DIR)/environment.c \
       $(SRC_DIR)/builtin.c \
       $(SRC_DIR)/interpreter.c

MAIN_SRC = $(SRC_DIR)/main.c
TEST_SRCS = $(TEST_DIR)/test_main.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MAIN_OBJ = $(OBJ_DIR)/main.o

.PHONY: all clean test debug run

all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@cp -f $@ $(BIN_DIR)/$@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: clean $(TARGET)

test: CFLAGS = $(DEBUG_CFLAGS)
test: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRCS) $(OBJS) $(LDFLAGS)
	@echo "=== Running MRT Test Suite ==="
	@./$(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) examples/hello.mrt

studio: $(TARGET)
	$(MAKE) -C mrt-studio

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TARGET)
	$(MAKE) -C mrt-studio clean
