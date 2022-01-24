#ifndef PRUNE_INDEX_H
#define PRUNE_INDEX_H

#include "constants.hpp"
#include "guess.hpp"

#include <boost/dynamic_bitset.hpp>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

class PruneIndex {
 public:
  PruneIndex(const std::vector<std::string>* wordlist)
    : wordlist_(wordlist), size_(wordlist->size()) {
      index();
    }

  PruneIndex(const std::vector<std::string>* wordlist, std::ifstream& file)
    : wordlist_(wordlist), size_(wordlist->size()) {
    _index_word_to_i();
    load(file);
  }

  ~PruneIndex(){}

  // TODO operator[] / .at access
  // TODO how can we handle out-of-set guesses? build up index on the fly?
  const boost::dynamic_bitset<>* prune(uint64_t gid) const;
  const boost::dynamic_bitset<>* prune(const Guess& guess) const;
  const boost::dynamic_bitset<>* prune(size_t i, size_t j) const;
  const boost::dynamic_bitset<>* prune(std::string g, std::string s) const;

  void save(std::ostream& os) const;

  const std::vector<std::string>& wordlist() const {
    return *wordlist_;
  }

  size_t size() const {
    return size_;
  }

  // TODO remove, just wordlist**2
  size_t num_guesses_ = 0;

  size_t num_prune_checks_ = 0;


 private:
  /**
   * Populate this object's index
   */
  void index();

  void _index_word_to_i();

  void _index_guess();

  void _index_prune();

  void load(std::ifstream& file);

  const std::vector<std::string>* const wordlist_;

  std::unordered_map<std::string, size_t> word_to_i_;

  /**
   * An index of all precomputed guess-solution ids, where guess_index_[g][s]
   * holds the Guess formed by guessing wordlist_[g] on wordlist_[s].
   */
  std::vector<std::vector<uint64_t>> guess_index_;

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

const boost::dynamic_bitset<>* PruneIndex::prune(const Guess& guess) const {
  return prune(guess.id_string());
}

const boost::dynamic_bitset<>* PruneIndex::prune(size_t i, size_t j) const {
  uint64_t gid = guess_index_.at(i).at(j);
  return prune(gid);
}

const boost::dynamic_bitset<>* PruneIndex::prune(std::string g, std::string s) const {
  return prune(word_to_i_.at(g), word_to_i_.at(s));
}

void PruneIndex::save(std::ostream& os) const {
  for (size_t i = 0; i < size_; ++i) {
    for (size_t j = 0; j < size_; ++j) {
      os << guess_index_.at(i).at(j) << "\n";
    }
  }
  os << "\n";
  for (const auto& [k,v] : prune_index_) {
    os << k << " " << v << "\n";
  }
  os << std::flush;
}

/**
 * Private
 */

void PruneIndex::index() {
  _index_guess();
  _index_prune();
  _index_word_to_i();
}

void PruneIndex::_index_guess() {
  guess_index_.resize(size_);

  for (size_t i = 0; i < size_; ++i) {
    std::string g = wordlist_->at(i);

    guess_index_[i].resize(size_);
    for (size_t j = 0; j < size_; ++j) {
      std::string s = wordlist_->at(j);
      Guess guess(g);
      guess.check(s, false);

      //std::cout << guess << std::endl;
      //guess.pprint_id();

      guess_index_[i][j] = guess.id_string();
      ++num_guesses_;
    }
  }
}

void PruneIndex::_index_prune() {
  for (size_t i = 0; i < size_; ++i) {
    std::vector<uint64_t>* guesses = &guess_index_.at(i);

    for (size_t j = 0; j < size_; ++j) {
      uint64_t gid = guesses->at(j);
      // Index all like guesses as a dynamic bitset
      if (prune_index_.count(gid)) {
        // This gid has been pruned
        continue;
      }

      prune_index_.insert({gid, boost::dynamic_bitset<>(size_, 0)});
      boost::dynamic_bitset<>* prune = &prune_index_.at(gid);

      // Check this gid against all other gids
      for (size_t k = 0; k < size_; ++k) {
        uint64_t sid = guesses->at(k);
        // If guesses match, then they would *not* be pruned.
        if (gid != sid) {
          (*prune)[k] = 1;
        }
        ++num_prune_checks_;
      }
    }
  }
}

void PruneIndex::_index_word_to_i() {
  for (size_t i = 0; i < size_; ++i) {
    std::string g = wordlist_->at(i);
    word_to_i_.insert({g, i});
  }
}

void PruneIndex::load(std::ifstream& file) {
  guess_index_.resize(size_);
  for (size_t i = 0; i < size_; ++i) {
    guess_index_.at(i).resize(size_);
    for (size_t j = 0; j < size_; ++j) {
      file >> guess_index_.at(i).at(j);
    }
  }

  // Make sure we're aligned with empty line
  std::string line;
  std::getline(file, line);
  assert(line.size() == 0);

  uint64_t gid;
  std::string bitstring;
  boost::dynamic_bitset<> bits;
  while(file >> gid >> bits) {
    prune_index_.insert({gid, bits});
  }
}

#endif
