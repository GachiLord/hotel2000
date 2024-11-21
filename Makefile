CFLAGS = `pkg-config --cflags gtk4` \
		`pkg-config --libs gtk4` \
		`pkg-config --libs libpq` \
		-lm \


run: build
	./target/hotel2000

build:
	clear
	mkdir -p target
	clang $(CFLAGS) \
		-o ./target/hotel2000 ./src/*.c \

debug:
	clear
	mkdir -p target
	clang $(CFLAGS) \
		-g \
		-o ./target/hotel2000 ./src/*.c \

anal:
	clear
	mkdir -p target
	clang $(CFLAGS) \
		./src/*.c \
		--analyze \
