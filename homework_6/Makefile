CC = gcc
CFLAGS = -Wall -Wextra
SRC = src
BUILD = build
SRCS = $(wildcard $(SRC)/*.c)
TARGETS = $(SRCS:$(SRC)/%.c=$(BUILD)/%)

.PHONY: all clean test

all: $(TARGETS)

$(BUILD)/%: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) test.txt output.txt redirect.txt input.txt __pipe_temp__
