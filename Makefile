SRC_DIR := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)


run: nbody
	./nbody

nbody: $(SRC_FILES)
	g++ $(SRC_FILES) -fopenmp -Ivendor -Isrc -o nbody
