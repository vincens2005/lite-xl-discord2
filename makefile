all: build

build:
	@echo building...

	if [ ! -d "build" ]; then \
		mkdir build; \
	fi

	gcc -Wall -Iinclude main.c -o build/discord -lm

	@echo done!

clean:
	rm -rf build

.PHONY: all clean
