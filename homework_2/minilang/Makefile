CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = minilang
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/hello.ml

test: $(TARGET)
	@echo "=== Testing Hello ==="
	./$(TARGET) tests/hello.ml
	@echo "=== Testing Factorial ==="
	./$(TARGET) tests/factorial.ml
	@echo "=== Testing Fibonacci ==="
	./$(TARGET) tests/fib.ml
	@echo "=== Testing Control Flow ==="
	./$(TARGET) tests/control.ml
	@echo "=== Testing Operators ==="
	./$(TARGET) tests/operators.ml

.PHONY: all clean run test
