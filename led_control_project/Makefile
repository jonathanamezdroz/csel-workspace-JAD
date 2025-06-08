SRC_DIR=./src
SRC_FILES=$(shell find $(SRC_DIR) -name '*.c')
BUILD_DIR=./build

INCL_SRC=-I$(SRC_DIR)/

CC=/buildroot/output/host/bin/aarch64-linux-gcc
CFLAGS=-Wall -Wextra -Werror

OBJS_SRC:=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

app_epoll: make_dir $(BUILD_DIR)/app_epoll
app_select: make_dir $(BUILD_DIR)/app_select
app_silly: make_dir $(BUILD_DIR)/app_silly

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INCL_SRC)

$(BUILD_DIR)/main_%.o: main_%.c
	$(CC) $(CFLAGS) -c -o $@ $^ $(INCL_SRC)

#todo (nth) : put wildcard for each app. For some reasons, {... app_% ... main_% ... } did not work
$(BUILD_DIR)/app_epoll: $(OBJS_SRC) $(BUILD_DIR)/main_epoll.o
	$(CC) $(CFLAGS) -o $@ $^
$(BUILD_DIR)/app_select: $(OBJS_SRC) $(BUILD_DIR)/main_select.o
	$(CC) $(CFLAGS) -o $@ $^
$(BUILD_DIR)/app_silly: $(OBJS_SRC) $(BUILD_DIR)/main_silly.o
	$(CC) $(CFLAGS) -o $@ $^

make_dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/*

.PHONY: all clean
