CC = gcc

MODE ?= release

CFLAGS = `pkg-config --cflags gtk4` \
		`pkg-config --libs gtk4` \
		`pkg-config --libs libpq` \
		-Wall -Wextra \
		-Wno-unused-parameter \
		-lm \

ifeq ($(MODE), release)
	CFLAGS += -O3
else
	CFLAGS += -fsanitize=address \
						-fsanitize=pointer-compare \
						-fsanitize=leak \
						-g -O1 \

endif

SRCS = $(wildcard ./*.c)

OBJS = $(SRCS:.c=.o)

# build

TARGET = hotel2000

run: all
	./$(TARGET)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# analyze

anal: all
	$(CC) $(CFLAGS) *.c --analyzer

clean:
	rm -f *.o *.out *.plist $(TARGET)

.PHONY: all clean
