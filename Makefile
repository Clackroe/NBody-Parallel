SRC_DIR := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)


display: run
	bash open simulation.gif &  python3 display.py

run: nbody
	./nbody

nbody: $(SRC_FILES)
	g++ $(SRC_FILES) -fopenmp -Ivendor -Isrc -o nbody
