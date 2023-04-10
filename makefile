all: build


build: main.c include/*
	@echo building discord rpc...

	if [ ! -d "build" ]; then \
		mkdir build; \
	fi

	gcc -Wall -Iinclude main.c -o build/discord -lm

	@echo done!

clean:
	rm -rf build
	cd raylib/src/ && \
	make clean

run: build
	./build/discord

.PHONY: all clean

