#ifndef WORD_H
#define WORD_H

#include "constants.hpp"

#include <bitset>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Word {
 public:
  Word(const std::string& word)
    : word_(word) {
    encode();
  }

  const std::string& get_word() const {
    return word_;
  }

  const uint8_t* get_letters() const {
    return letters_;
  }

  const uint8_t* get_letter_counts() const {
    return letter_counts_;
  }

  const std::unordered_set<uint8_t>& get_letterset(std::bitset<5> key) const {
    return lettersets_.at(key);
  }

  const std::vector<uint8_t>& get_indices_of(uint8_t letter) const {
    assert(letter < 26);
    return reverse_letter_index_[letter];
  }

 private:
  void encode();

  void populate_lettersets();

  std::string word_;

  /*
   * Store letters as a char array for green letter comparisons.
   */
  uint8_t letters_[5];

  /**
   * Count of each letter, where letter_counts_[letter] is the number of times
   * the letter occurs in letters_.
   */
  uint8_t letter_counts_[26];

  /**
   * For every combination of placed letters, store a set of the remaining
   * letters for use in yellow checks.
   */
  std::unordered_map<std::bitset<5>, std::unordered_set<uint8_t>> lettersets_;

  /**
   * Index where reverse_letter_index_[letter] is a vector containing the
   * index locations i of the letter such that letters[i] = letter, in
   * ascending order.
   * Used during guess-pair construction to save time construction a 'placed
   * tiles' bitset used as a key to lettersets_.
   */
  std::vector<std::vector<uint8_t>> reverse_letter_index_;
};

void Word::encode() {

  memset(letter_counts_, 0, 26);
  reverse_letter_index_.resize(26);

  for (uint8_t i = 0; i < 5; ++i) {
    uint8_t letter = (uint8_t) (word_[i] - 'a');
    letters_[i] = letter;
    ++letter_counts_[letter];
    assert(letter < 26);
    reverse_letter_index_[letter].push_back(i);
  }

  populate_lettersets();
}

void Word::populate_lettersets() {
  for (uint8_t i = 0; i < 32; ++i) {
    std::bitset<5> key(i);

    std::unordered_set<uint8_t> letterset;
    for (size_t i = 0; i < key.size(); ++i) {
      bool placed = key[i];

      if (!placed) {
        letterset.insert(letters_[i]);
      }
    }
    lettersets_.insert({key, letterset});
  }
}

#endif
