#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "constants.hpp"
#include "guess.hpp"

#include <assert.h>

#include <bitset>
#include <iostream>
#include <map>
#include <stack>
#include <vector>

const size_t BITS_PER_LETTER = 5;
const size_t BITS_PER_COUNT = 2;
const uint64_t LSB_MASK = 0x5555555555555555;
const uint64_t MSB_MASK = 0xAAAAAAAAAAAAAAAA;

class Dictionary {
 public:
  Dictionary(const std::vector<std::string>& wordlist)
    : reference_words(wordlist){
    encode_wordlist();
  }

  ~Dictionary() {
    while (pruned_stack_.size()) {
      delete pruned_stack_.top();
      pruned_stack_.pop();
    }
  }

  /**
   * Prune dictionary using the inferences made in guess.
   *
   * Adds a pruned vector to pruned_stack_.
   * Returns a pointer to this.pruned_ after pruning the given guess.
   */
  std::vector<bool>* prune(const Guess& guess);

  void pop();

  size_t size() const;

  size_t count() const;

  const std::vector<std::string> reference_words;

  bool is_pruned(size_t i) const {
    return pruned_->at(i);
  }

  // TODO
  const std::vector<bool>& key() const {
    return *pruned_;
  }

 private:
  /**
   * Returns the final borrow bit of 2-element-wise x - y.
   */
  static uint64_t borrow_2bit(uint64_t x, uint64_t y);

  bool should_prune_word(uint32_t encoded_word, uint64_t letter_cts,
                         uint32_t c_check, uint32_t c_mask,
                         uint32_t w_check, uint32_t w_mask,
                         uint64_t min_cts,
                         uint64_t max_cts, uint64_t max_mask);

  void encode_wordlist();

  /**
   * For each word, store an encoded representation of the letter + positions,
   * as well as a 2-bit letter count for each letter and a 'pruned' vector to
   * denote whether a word should be considered pruned from the dataset.
   */
  std::vector<bool>* pruned_;
  std::stack<std::vector<bool>*> pruned_stack_;

  std::vector<uint32_t> words_;
  std::vector<uint64_t> counts_;
};


// TODO
/**
 * Public implementations
 */
//void Dictionary::reset_to(std::vector<bool>* snapshot) {
//  std::vector<bool>* tmp = pruned_;
//  pruned_ = new std::vector<bool>(*snapshot);
//  delete tmp;
//  //pruned_ = pruned; // TODO
//}

//std::vector<bool> Dictionary::snapshot() {
//  return std::vector<bool>(*pruned_);
//}

std::vector<bool>* Dictionary::prune(const Guess& guess) {
  pruned_ = new std::vector<bool>(*pruned_); // old reference still in stack
  pruned_stack_.push(pruned_);

  /**
   * Prune based on correct placements.
   * Anything that does not match all correct placements will be pruned.
   *
   * Assemble a check of all correct placements and XOR with word.
   * For guess share on solve, we have correct letters (0,s), (4,e):
   *                 e                   s
   *      00000000010000000000000000010010
   *  XOR                   (encoded word)
   *  AND 00000001111100000000000000011111
   *
   *  any bits mean there was a mismatch => prune
   */
  // TODO consider caching check+mask in Dictionary
  uint32_t c_check = 0;
  uint32_t c_mask = 0;
  for (const auto& [pos, l] : guess.correct_placements) {
    c_check |= ((uint32_t) l - 'a') << BITS_PER_LETTER * pos;
    c_mask |= (uint32_t) 0b11111 << BITS_PER_LETTER * pos;
  }

  //std::cout << std::bitset<32>(c_check) << std::endl;
  //std::cout << std::bitset<32>(c_mask) << std::endl;
  //std::cout << std::bitset<32>(c_result) << std::endl;

  /**
   * Prune based on incorrect placements.
   * Anything that matches a placement will be pruned.
   *
   * We perform the same check and mask as for correct placements, but now
   * prune if any blocks (letters) match. We cannot do this in a single
   * bitwise operation and have to check each block individually (AFAICT).
   *
   *                 e                   s
   *      00000000010000000000000000010010
   *  XOR                   (encoded word)
   *  OR  11111110000011111111111111100000
   *
   *  if any block == 00000, there was a match => prune
   */
  uint32_t w_check = 0;
  uint32_t w_mask = 0xFFFFFFFF;
  for (const auto& [pos, l] : guess.wrong_placements) {
    w_check |= ((uint32_t) l - 'a') << BITS_PER_LETTER * pos;
    w_mask ^= (uint32_t) 0b11111 << BITS_PER_LETTER * pos;
  }

  /**
   * Prune based on minimum letter count.
   * Any letter that has less than the minimum letter count will be pruned.
   *
   * We can perform a partial 2-bit subtraction on each separate letter block
   * to get the the sign bit of the subtraction result.
   *
   * We take the borrow bit of a half subtractor from subtracting the first
   * digit of each block, and then compute the borrow bit of a full subtractor
   * for the second digit of each block.
   *
   * We encode the minimum letter checks the same way as the letter counts.
   * (a,2), (s,1) is encoded as:
   *                                                               (count)
   *                                1s                                  2a
   *  -   0000000000000000000000000001000000000000000000000000000000000010
   *
   * If any 1s, then the result of the subtraction is negative => prune
   */
  uint64_t min_cts = 0;
  for (const auto& [l, ct] : guess.min_letter_counts) {
    min_cts |= (uint64_t) ct << BITS_PER_COUNT * ((uint8_t) l - 'a');
  }

  /**
   * Prune based on maximum letter count.
   * Any letter that has more than the maximum letter count will be pruned.
   *
   * We perform the same subtraction method in the opposite order to get the
   * sign bit of:
   * max_letter_ct - (count)
   */
  uint64_t max_cts = 0;
  uint64_t max_mask = 0;
  for (const auto& [l, ct] : guess.max_letter_counts) {
    max_cts |= (uint64_t) ct << BITS_PER_COUNT * ((uint8_t) l - 'a');
    max_mask |= (uint64_t) 0b11 << BITS_PER_COUNT * ((uint8_t) l - 'a');
  }

  // Prune wordset
  for (size_t i = 0; i < pruned_->size(); ++i) {
    if (pruned_->at(i)) {
      // This element is already pruned, skip.
      continue;
    }

    const uint32_t encoded_word = words_[i];
    const uint64_t letter_cts = counts_[i];

    pruned_->at(i) = should_prune_word(encoded_word, letter_cts,
                                             c_check, c_mask,
                                             w_check, w_mask,
                                             min_cts,
                                             max_cts, max_mask);
  }

  return pruned_;
}

void Dictionary::pop() {
  delete pruned_;
  pruned_stack_.pop();
  pruned_ = pruned_stack_.top();
}

size_t Dictionary::size() const {
  return reference_words.size();
}

size_t Dictionary::count() const {
  // TODO optimize with saved var
  size_t sum = 0;
  for (bool p : *pruned_) {
    sum += !p;
  }
  return sum;
}

/**
 * Private implementations
 */

uint64_t Dictionary::borrow_2bit(uint64_t x, uint64_t y) {
  // Get LSB borrow bit
  /**
   * This matches any 0/1 x/y pairs in the first digit.
   */
  uint64_t tmp = (~x & y);
  uint64_t b_in = LSB_MASK & tmp;

  /**
   * This matches any 0/1 pairs in the second digit:
   *     (~x & y)
   *   OR
   * any matching digits
   *     ~(x ^ y)
   *   with a borrow bit from the previous digit:
   *     (b_in << 1)
   */
  return MSB_MASK & (tmp | ((b_in << 1) & ~(x ^ y)));
}

bool Dictionary::should_prune_word(uint32_t encoded_word, uint64_t letter_cts,
                                   uint32_t c_check, uint32_t c_mask,
                                   uint32_t w_check, uint32_t w_mask,
                                   uint64_t min_cts,
                                   uint64_t max_cts, uint64_t max_mask) {


    // Check correct placements
    uint32_t c_result = (c_check ^ encoded_word) & c_mask;
    if (c_result) {
      //std::cout << "Pruned for not having correct placements." << std::endl;
      return true;
    }

    // Check wrong placements
    uint32_t w_result = (w_check ^ encoded_word) | w_mask;
    for (uint8_t pos = 0; pos < NUM_LETTERS; ++pos) {
      uint8_t block = (w_result >> BITS_PER_LETTER * pos) & 0b11111;
      if (!block) {
        //std::cout << "Pruned for having wrong placement." << std::endl;
        return true;
      }
    }

    // Check min letter count
    uint64_t min_result = borrow_2bit(letter_cts, min_cts);
    if (min_result) {
      //std::cout << "Pruned for not meeting min letters." << std::endl;
      return true;
    }

    // Check max letter count
    uint64_t max_result = max_mask & borrow_2bit(max_cts, letter_cts);
    if (max_result) {
      //std::cout << "Pruned for having over max letters." << std::endl;
      return true;
    }

    return false;
}

void Dictionary::encode_wordlist() {
  pruned_ = new std::vector<bool>(reference_words.size(), false);
  pruned_stack_.push(pruned_);

  for (std::string word : reference_words) {
    /**
     * Encode word as a sequence of five 5-bit numbers
     * --> 25 bits, 7 bits padding
     *
     * adult is encoded as:
     * -------    t    l    u    d    a
     * 00000001001101011101000001100000
     */
    uint32_t encoded_word = 0;
    for (uint8_t i = 0; i < NUM_LETTERS; ++i) {
      uint8_t c = (uint8_t) word[i] - 'a';
      encoded_word |= (uint32_t) c << BITS_PER_LETTER * i;
    }
    words_.push_back(encoded_word);

    /**
     * Encode letter counts as 2-bit count per letter
     * --> 52 bits, 12 bits padding
     *
     * aorta is encoded as:
     * ------------            1t  1r    1o                          2a
     * 0000000000000000000000000100010000010000000000000000000000000010
     */
    std::map<char, int> l_counts;
    for (char c : word) {
      ++l_counts[c];
    }
    uint64_t encoded_count = 0;
    for (const auto& [l, ct] : l_counts) {
      uint8_t pos = (uint8_t) l - 'a';
      encoded_count |= (uint64_t) ct << BITS_PER_COUNT * pos;
    }
    counts_.push_back(encoded_count);
  }
}

#endif
