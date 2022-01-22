CXX := g++
CXXFLAGS := -std=c++17 -g -O2 -Wall -Wextra -Wconversion -pedantic -MD -MP

SRC_DIR := src
OBJ_DIR := build
EXE := wordle_bits

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPFILES := $(OBJECTS:.o=.d)


all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $(EXE) $(LIBS)

-include $(DEPFILES)

run: all
	time ./wordle_bits config/solution_words.txt

sorted: all
	time ./wordle_bits config/solution_size_srtd.txt

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) wordle_bits

.PHONY: all run clean
