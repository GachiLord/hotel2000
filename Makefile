run: build
	./target/hotel2000

build:
	mkdir -p target
	gcc `pkg-config --cflags gtk4` -o ./target/hotel2000 ./src/main.c `pkg-config --libs gtk4`
