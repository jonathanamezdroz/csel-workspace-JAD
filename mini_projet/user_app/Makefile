SRC_DIR=./src
SRC_FILES=$(shell find $(SRC_DIR) -name '*.c')
BUILD_DIR=./build

INCL_SRC=-I$(SRC_DIR)/

CC=/buildroot/output/host/bin/aarch64-linux-gcc
CFLAGS=-Wall -Wextra -Werror

OBJS_SRC:=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

app: make_dir $(BUILD_DIR)/app

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INCL_SRC)

$(BUILD_DIR)/main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $^ $(INCL_SRC)

$(BUILD_DIR)/app: $(OBJS_SRC) $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

make_dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/*

.PHONY: all clean
