CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS =
LIBS =

TARGET = custom_shell
SRCS = shell.c linenoise.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
