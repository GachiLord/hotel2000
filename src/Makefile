CC = clang

CFLAGS = `pkg-config --cflags gtk4` \
		`pkg-config --libs gtk4` \
		`pkg-config --libs libpq` \
		-Wall -Wextra \
		-lm \

SRCS = $(wildcard ./*.c)

OBJS = $(SRCS:.c=.o)

TARGET = hotel2000

run: all
	./hotel2000

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

anal: all
	$(CC) $(CFLAGS) --analyze 

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
