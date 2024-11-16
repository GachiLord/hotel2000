CFLAGS = `pkg-config --cflags gtk4` \
		`pkg-config --libs gtk4` \
		`pkg-config --libs libpq` \


run: build
	./target/hotel2000

build:
	clear
	mkdir -p target
	gcc $(CFLAGS) \
		-o ./target/hotel2000 ./src/* \

debug:
	clear
	mkdir -p target
	gcc $(CFLAGS) \
		-g \
		-o ./target/hotel2000 ./src/* \
