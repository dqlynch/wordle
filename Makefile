CXX := g++
CXXFLAGS := -std=c++17 -g -O2 -Wall -Wextra -Wconversion -pedantic -MD -MP

SRC_DIR := src
OBJ_DIR := build
EXE := wordle_bits

#LDLIBS := -lboost_system

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPFILES := $(OBJECTS:.o=.d)


all: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $(EXE) $(LDLIBS) -v

-include $(DEPFILES)

run: all
	time ./wordle_bits config/solution_words.txt config/solution_words.pindex

sorted: all
	time ./wordle_bits config/solution_size_srtd.txt


$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) wordle_bits

.PHONY: all run clean
