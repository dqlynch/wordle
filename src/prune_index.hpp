#ifndef PRUNE_INDEX_H
#define PRUNE_INDEX_H

#include "constants.hpp"
#include "guess_pair.hpp"
#include "guess_pair_index.hpp"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

const size_t SIZE_UL = sizeof(unsigned long);
const size_t SIZE_64 = sizeof(uint64_t);

class PruneIndex {
 public:
  PruneIndex(const std::vector<std::string>& wordlist)
    : guess_index_(GuessPairIndex(wordlist)), size_(wordlist.size()) {
    index();
  }

  PruneIndex(const PruneIndex&) = delete;
  PruneIndex(PruneIndex&&) = default;

  PruneIndex(const std::vector<std::string>& wordlist, const std::string& filename)
    : guess_index_(GuessPairIndex(wordlist)), size_(wordlist.size()) {
    load_or_generate(filename);
  }

  ~PruneIndex(){}

  // TODO how can we handle out-of-set guesses? build up index on the fly?
  const boost::dynamic_bitset<>* prune(uint64_t gid) const;
  //const boost::dynamic_bitset<>* prune(const Guess& guess) const; // TODO
  const boost::dynamic_bitset<>* prune(size_t i, size_t j) const;

  void save(std::ostream& os) const;

  size_t size() const {
    return size_;
  }

  void _dump() const {
    for (const auto& [gid,v] : prune_index_) {
      std::cout << gid << " " << v << std::endl;
    }

    std::cout << prune_index_.size() << std::endl;
  }

 private:
  /**
   * Populate this object's index
   */
  void index();

  void load(std::ifstream& file);

  void load_or_generate(const std::string& filename);

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

/**
 * Private
 */

void PruneIndex::index() {
  _index_prune();
}

// TODO this is slow, save to file
void PruneIndex::_index_prune() {
  for (size_t i = 0; i < size_; ++i) {
    const std::vector<uint64_t>& g_pairs = guess_index_[i];

    for (size_t j = 0; j < size_; ++j) {
      uint64_t gid = g_pairs[j];
      // Index all like g guess-pairs as a dynamic bitset
      if (prune_index_.count(gid)) {
        // This gid has already been computed
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

void PruneIndex::save(std::ostream& os) const {
  boost::dynamic_bitset<> ulong_mask(size_, ULONG_MAX);

  // Write size of guess id keyset as fixed size 64-bit uint
  uint64_t k_size = prune_index_.size();
  os.write(reinterpret_cast<char *>(&k_size), SIZE_64);

  for (auto [gid, bits] : prune_index_) {
    // Write 64-bit guess id
    os.write(reinterpret_cast<const char*>(&gid), SIZE_64);

    // Write bitset in chunks of size ulong
    // TODO can technically assemble an in-mem buffer and write all at once,
    // but only really load needs to be fast once pindex is generated
    for (size_t i = 0; i < size_; i += 8 * SIZE_UL) {
      unsigned long ulong = (bits & ulong_mask).to_ulong();

      os.write(reinterpret_cast<char*>(&ulong), SIZE_UL);

      bits >>= (8 * SIZE_UL);
    }
  }
}

void PruneIndex::load(std::ifstream& file) {
  // Number of unsigned long blocks required to fully hold size_ bits
  const size_t UL_BLOCKS_PER_BITSET = (size_ - 1) / (8 * SIZE_UL) + 1;

  char buf_64[SIZE_64]; // separate buffer for reading fixed size vals

  char* bits_buf = new char[UL_BLOCKS_PER_BITSET * SIZE_UL];
  unsigned long* block_itr = reinterpret_cast<unsigned long*>(bits_buf);
  unsigned long* block_itr_end = block_itr + UL_BLOCKS_PER_BITSET;

  // Read fixed length size of guess id keyset
  uint64_t k_size;
  file.read(buf_64, SIZE_64);
  memcpy(&k_size, buf_64, SIZE_64);

  for (size_t i = 0; i < k_size; ++i) {
    // Read fixed length gid key
    uint64_t gid;
    file.read(buf_64, SIZE_64);
    memcpy(&gid, buf_64, SIZE_64);

    /**
     * We have to construct the bitset in a way that avoids single-bit
     * assignments to keep this fast: builtin stream operator reads bit
     * by bit and takes forever.
     * Read file binary into contiguous blocks of ulong and pass iterator
     * directly to dynamic::bitset constructor.
     */
    boost::dynamic_bitset<> bits;
    file.read(bits_buf, (long) (SIZE_UL * UL_BLOCKS_PER_BITSET));
    bits.append(block_itr, block_itr_end);
    bits.resize(size_);   // trim any excess bits from last ulong block

    prune_index_.insert({gid, bits});
  }

  delete[] bits_buf;
}

void PruneIndex::load_or_generate(const std::string& filename) {
  std::ifstream file(filename);
  if (file.good()) {
    load(file);
  } else {
    file.close();

    std::ofstream out(filename);
    index();
    save(out);
  }
}

#endif
