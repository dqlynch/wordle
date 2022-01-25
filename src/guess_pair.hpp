#ifndef GUESS_PAIR_H
#define GUESS_PAIR_H

#include "constants.hpp"
#include "word.hpp"

#include <bitset>

const uint8_t YELLOW = 0b01;
const uint8_t GREEN = 0b10;

class GuessPair {
 public:
  GuessPair(const Word& g, const Word& s)
    : guess_(g), solution_(s) {
    compute_id();
  }

  uint64_t id() const {
    return guess_id_;
  }

  const GuessPair& test() {
    return *this;
  }

 private:
  void compute_id();

  const Word& guess_;
  const Word& solution_;

  uint64_t guess_id_;
};

void GuessPair::compute_id() {
  uint64_t gid = 0;

  bool greens[5];
  bool yellows[5];
  memset(greens, 0, 5);
  memset(yellows, 0, 5);

  // The count of each letter we've placed. We need this so that we can
  // handle scenarios where the guess and solution have differing non-zero
  // counts of the same letter.
  uint8_t placed_letter_counts[26];
  memset(placed_letter_counts, 0, 26);

  // Arrays of the letters in the guess and solution.
  const uint8_t* g_letters = guess_.get_letters();
  const uint8_t* s_letters = solution_.get_letters();
  const uint8_t* s_letter_counts = solution_.get_letter_counts();

  // Check for matching letters for greens *first*.
  for (uint8_t i = 0; i < 5; ++i) {
    uint8_t g = g_letters[i];
    gid |= (uint64_t) g << 7*i;

    if (g == s_letters[i]) {
      greens[i] = true;
      ++placed_letter_counts[g];

      // Place green in id string
      gid |= (uint64_t) GREEN << (7*i + 5);
    }
  }

  // Check remaining letters for yellow/greys
  for (uint8_t i = 0; i < 5; ++i) {
    if (greens[i]) {
      continue;
    }

    //const auto& letterset = solution_.get_letterset(key);

    //std::cout << key << std::endl;
    //for (uint8_t c : letterset) {
    //  std::cout << (char) ((char)c + 'a');
    //}
    //std::cout << std::endl;

    uint8_t g = g_letters[i];
    if (placed_letter_counts[g] < s_letter_counts[g]) {
      // Place yellow in id string
      gid |= (uint64_t) YELLOW << (7*i + 5);
      yellows[i] = true;

      ++placed_letter_counts[g];
      //std::cout << "y" << std::endl;
    }
    //if (letterset.count(g)) {
    //  // Find the first location in the *solution* of the letter we just placed
    //  // TODO
    //  uint8_t s_i = solution_.get_indices_of(g).at(placed_letter_counts[g]);
    //  std::cout << "s_i: " << (int) s_i << std::endl;

    //  yellows[i] = true;
    //  key[s_i] = true;
    //  ++placed_letter_counts[g];
    //  std::cout << "y" << std::endl;
    //}

    // Implicitly this tile is grey
    //std::cout << std::endl;
  }

  //for (uint8_t i = 0; i < 5; ++i) {
  //  std::cout << greens[i] << yellows[i] << " ";
  //}
  //std::cout << std::endl;
  guess_id_ = gid;
}

#endif
