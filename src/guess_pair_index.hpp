#ifndef GUESS_PAIR_INDEX_H
#define GUESS_PAIR_INDEX_H

#include "guess_pair.hpp"
#include "word.hpp"

/**
 * Precompute the guess-pair ids for all guess-pairs in the given wordlist.
 */
class GuessPairIndex {
 public:
  GuessPairIndex(const std::vector<std::string>& wordlist) {
    index(wordlist);
  }

  const std::vector<uint64_t>& operator[](size_t i) const {
    return guess_index_[i];
  }

  size_t size() const {
    return size_;
  }

 private:
  void index(const std::vector<std::string>& wordlist);

  std::vector<Word> words_;

  std::vector<std::vector<uint64_t>> guess_index_;

  size_t size_ = 0;
};

void GuessPairIndex::index(const std::vector<std::string>& wordlist) {
  size_t subsize = wordlist.size();

  words_.reserve(subsize);
  for (std::string w : wordlist) {
    words_.push_back(Word(w));
  }

  guess_index_.resize(subsize);

  for (size_t i = 0; i < subsize; ++i) {
    guess_index_[i].resize(subsize);

    for (size_t j = 0; j < subsize; ++j) {
      GuessPair gp(words_[i], words_[j]);
      guess_index_[i][j] = gp.id();
    }
  }

  size_ = subsize * subsize;
}

#endif
