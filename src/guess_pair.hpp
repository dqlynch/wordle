#ifndef GUESS_PAIR_H
#define GUESS_PAIR_H

#include "constants.hpp"
#include "word.hpp"

#include <bitset>

const uint8_t YELLOW = 0b01;
const uint8_t GREEN = 0b10;

class GuessPair {
 public:
  GuessPair(const Word& g, const Word& s) {
    compute_id(g, s);
  }

  GuessPair(const GuessPair&) = delete;

  uint64_t id() const {
    return guess_id_;
  }

  const GuessPair& test() {
    return *this;
  }

 private:
  void compute_id(const Word& guess, const Word& solution);

  uint64_t guess_id_;
};

void GuessPair::compute_id(const Word& guess, const Word& solution) {
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
  const uint8_t* g_letters = guess.get_letters();
  const uint8_t* s_letters = solution.get_letters();
  const uint8_t* s_letter_counts = solution.get_letter_counts();

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

    uint8_t g = g_letters[i];
    if (placed_letter_counts[g] < s_letter_counts[g]) {
      // Place yellow in id string
      gid |= (uint64_t) YELLOW << (7*i + 5);
      yellows[i] = true;

      ++placed_letter_counts[g];
    }
  }
  guess_id_ = gid;
}

#endif
