CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wconversion -pedantic -MD -MP
FFLAGS := -O3 -funroll-loops -DNDEBUG
#FFLAGS := -Og -g

SRC_DIR := src
OBJ_DIR := build
EXE := wordle_bits

#LDLIBS := -lboost_system

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPFILES := $(OBJECTS:.o=.d)


all: $(OBJECTS)
	$(CXX) $(FFLAGS) $(LDFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $(EXE) $(LDLIBS) -v

-include $(DEPFILES)

run: all
	time ./wordle_bits config/solution_words.txt pindex/solution_words.pindex

small: all
	time ./wordle_bits config/small.txt pindex/small.pindex

guess: all
	time ./wordle_bits config/additional_guess_words.txt pindex/additional_guess_words.pindex

allwords: all
	time ./wordle_bits config/all_words.txt pindex/all_words.pindex


$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) $(FFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) wordle_bits

.PHONY: all run clean
