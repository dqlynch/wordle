#ifndef PRUNE_INDEX_H
#define PRUNE_INDEX_H

#include "constants.hpp"
#include "guess_pair.hpp"
#include "guess_pair_index.hpp"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

class PruneIndex {
 public:
  PruneIndex(const std::vector<std::string>& wordlist)
    : guess_index_(GuessPairIndex(wordlist)), size_(wordlist.size()) {
      index(wordlist);
    }

  ~PruneIndex(){}

  // TODO how can we handle out-of-set guesses? build up index on the fly?
  const boost::dynamic_bitset<>* prune(uint64_t gid) const;
  //const boost::dynamic_bitset<>* prune(const Guess& guess) const; // TODO
  const boost::dynamic_bitset<>* prune(size_t i, size_t j) const;
  const boost::dynamic_bitset<>* prune(std::string g, std::string s) const;

  void save(std::ostream& os) const;

  size_t size() const {
    return size_;
  }

 private:
  /**
   * Populate this object's index
   */
  void index(const std::vector<std::string>& wordlist);

  void _index_word_to_i(const std::vector<std::string>& wordlist);

  void _index_prune();

  GuessPairIndex guess_index_;

  std::unordered_map<std::string, size_t> word_to_i_;

  std::unordered_map<uint64_t, boost::dynamic_bitset<>> prune_index_;

  const size_t size_;
};

/**
 * Public
 */
const boost::dynamic_bitset<>* PruneIndex::prune(uint64_t gid) const {
  assert(prune_index_.count(gid));
  return &prune_index_.at(gid);
}

// Maybe useful for display?
//const boost::dynamic_bitset<>* PruneIndex::prune(const Guess& guess) const {
//  return prune(guess.id_string());
//}

const boost::dynamic_bitset<>* PruneIndex::prune(size_t i, size_t j) const {
  uint64_t gid = guess_index_[i][j];
  return prune(gid);
}

const boost::dynamic_bitset<>* PruneIndex::prune(std::string g, std::string s) const {
  return prune(word_to_i_.at(g), word_to_i_.at(s));
}

/**
 * Private
 */

void PruneIndex::index(const std::vector<std::string>& wordlist) {
  std::cout << "Constructing prune vectors..." << std::endl;
  _index_prune();
  std::cout << "Done" << std::endl;
  _index_word_to_i(wordlist); // TODO this doesn't belong in this class
}

// TODO this is slow, save to file
void PruneIndex::_index_prune() {
  for (size_t i = 0; i < size_; ++i) {

    const std::vector<uint64_t>& g_pairs = guess_index_[i];

    for (size_t j = 0; j < size_; ++j) {
      uint64_t gid = g_pairs[j];
      // Index all like g guess-pairs as a dynamic bitset
      if (prune_index_.count(gid)) {
        // This gid has been pruned
        continue;
      }

      prune_index_.insert({gid, boost::dynamic_bitset<>(size_, 0)});
      boost::dynamic_bitset<>* prune = &prune_index_.at(gid);

      // Check this gid against all other gids
      for (size_t k = 0; k < size_; ++k) {
        uint64_t sid = g_pairs[k];
        // If g_pairs *don't* match, then they would be pruned.
        if (gid != sid) {
          (*prune)[k] = 1;
        }
      }
    }
  }
}

void PruneIndex::_index_word_to_i(const std::vector<std::string>& wordlist) {
  for (size_t i = 0; i < size_; ++i) {
    std::string g = wordlist.at(i);
    word_to_i_.insert({g, i});
  }
}

#endif
