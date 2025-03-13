EXE = ./build/ACG

.PHONY: all
all: build
	make -C build all

.PHONY: build
build:
	cmake -S . -B build

.PHONY: test
test: all
	$(EXE)
