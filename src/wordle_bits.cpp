#include "dictionary.hpp"
#include "guess.hpp"
#include "guess_pair.hpp"
#include "guess_pair_index.hpp"
#include "prune_index.hpp"
#include "solver.hpp"
#include "word.hpp"

#include <assert.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * Load wordlist from file.
 */
std::vector<std::string> load_wordlist(std::string filename) {
  std::ifstream file(filename);

  std::vector<std::string> wordlist;
  std::string word;
  while(file >> word) {
    wordlist.push_back(word);
  }

  return wordlist;
}

double test_guess_prune_shortcut(Dictionary* dict, std::string g) {
  std::vector<bool> computed(dict->reference_words.size(), 0);
  std::vector<int> sizes;

  for (unsigned int i = 0; i < dict->reference_words.size(); ++i) {
    if (computed[i]) {
      continue;
    }
    std::string s = dict->reference_words[i];

    Guess guess(g);
    guess.check(s);

    std::vector<bool>* pruned = dict->prune(guess);

    int sol_size = 0;
    for (unsigned int j = 0; j < pruned->size(); ++j) {
      if (!pruned->at(j)) {
        computed[j] = 1;
        ++sol_size;
      }
    }

    dict->pop();

    sizes.push_back(sol_size * sol_size);
  }


  double acc = 0;
  for (int size : sizes) {
    acc += size;
  }
  acc /= dict->reference_words.size();
  return acc;
}

std::vector<std::pair<std::string, double>> perf_test(std::vector<std::string> wordlist) {
  Dictionary dict(wordlist);

  std::vector<std::pair<std::string, double>> average_sizes;

  for (std::string g : wordlist) {
    double size = test_guess_prune_shortcut(&dict, g);
    average_sizes.push_back(std::pair<std::string, double>(g, size));
    //average_sizes.push_back(std::pair<std::string, double>(g, test_guess(dict, g)));
  }

  //for (const auto& [k,v] : average_sizes) {
  //  std::cout << k << "," << v << std::endl;
  //}

  std::sort(average_sizes.begin(), average_sizes.end(),
      [](const auto& a, const auto& b) {
        return a.second < b.second;
      });
  return average_sizes;
}

size_t count_zeros(const boost::dynamic_bitset<>& bits) {
  size_t ct = 0;
  for (size_t i = 0; i < bits.size(); ++i) {
    ct += !bits[i];
  }
  return ct;
}

double test_guess_index(const std::vector<std::string>& wordlist,
                        const PruneIndex& index,
                        const std::string& g,
                        const boost::dynamic_bitset<>& pruned) {
  boost::dynamic_bitset<> computed(pruned);

  size_t acc = 0;
  for (size_t i = 0; i < index.size(); ++i) {
    if (computed[i]) {
      continue;
    }
    std::string s = wordlist[i];

    const boost::dynamic_bitset<>* s_pruned = index.prune(g, s);

    computed |= ~*s_pruned;

    size_t size = count_zeros(*s_pruned | pruned);
    acc += size * size;
  }

  return (double) acc / (double) count_zeros(pruned);
}

void index_perf_test(const std::vector<std::string>& wordlist, const PruneIndex& index, const boost::dynamic_bitset<>& pruned) {
  std::vector<std::pair<std::string, double>> average_sizes;

  for (size_t i = 0; i < index.size(); ++i) {
    std::string g = wordlist[i];
    if (pruned[i]) {
      continue;
    }
    double size = test_guess_index(wordlist, index, g, pruned);
    average_sizes.push_back(std::pair<std::string, double>(g, size));
  }

  std::sort(average_sizes.begin(), average_sizes.end(),
      [](const auto& a, const auto& b) {
        return a.second < b.second;
      });

  //for (size_t i = 0; i < average_sizes.size(); ++i) {
  for (size_t i = 0; i < average_sizes.size() && i < 15; ++i) {
    auto& avg = average_sizes[i];
    std::cout << avg.first << " " << avg.second << std::endl;
  }
  //return average_sizes;
}

void index_perf_test(const std::vector<std::string>& wordlist) {
  const PruneIndex pindex(wordlist);
  index_perf_test(wordlist, pindex, boost::dynamic_bitset<>(wordlist.size(), 0));
}

//void index_helper_test(const PruneIndex& index) {
//  boost::dynamic_bitset<> pruned(index.size(), 0);
//
//  while (true) {
//    index_perf_test(index, pruned);
//
//    std::cout << "Input guess word: " << std::flush;
//    std::string g;
//    std::cin >> g;
//    Guess guess(g);
//
//    std::cout << "Input colors xyg: " << std::flush;
//    std::string colors;
//    std::cin >> colors;
//    guess.set(colors);
//    std::cout << std::endl;
//
//    pruned |= *index.prune(guess);
//    size_t remaining = count_zeros(pruned);
//    std::cout << remaining << " remaining." << std::endl;
//    if (remaining <= 1) {
//      for (size_t i = 0; i < index.size(); ++i) {
//        if (!pruned[i]) {
//          std::cout << index.wordlist()[i] << std::endl;
//          return;
//        }
//      }
//    }
//  }
//}

//void prune_index_test(const std::vector<std::string>& wordlist) {
//  // Initially prune for a smaller test set
//  Dictionary dict(wordlist);
//  //Guess guess("raise", "aural");   // --> 78 words
//  //Guess guess("jujus", "share");    // >> 466 words
//  //dict.prune(guess);
//
//  std::vector<std::string> pruned_wordlist;
//  for (size_t i = 0; i < wordlist.size(); ++i) {
//    if (!dict.is_pruned(i)) {
//      pruned_wordlist.push_back(wordlist.at(i));
//    }
//  }
//
//  //PruneIndex save_idx(&wordlist);
//  //PruneIndex index(&wordlist);
//
//  // Save test
//  //std::ofstream of("solution_words.pindex");
//  //save_idx.save(of);
//  //exit(0);
//
//  std::cout << "loading..." << std::endl;
//  std::ifstream file("solution_words.pindex");
//  PruneIndex index(&pruned_wordlist, file);
//  std::cout << "loaded..." << std::endl;
//
//  std::cout << index.num_guesses_ << std::endl;
//  std::cout << index.num_prune_checks_ << std::endl;
//
//  // Check traditionally
//  Guess cmp("raise", "torso");
//  dict.prune(cmp);
//
//  for (size_t i = 0; i < dict.size(); ++i) {
//    if (!dict.is_pruned(i)) {
//      std::cout << dict.reference_words.at(i) << " ";
//    }
//  }
//  std::cout << std::endl;
//  std::cout << dict.count() << std::endl;
//
//  dict.pop();
//
//  // Check with PruneIndex
//  const boost::dynamic_bitset<>* idx_pruned = index.prune(cmp);
//  std::cout << *idx_pruned << std::endl;
//
//  size_t p_idx = 0;
//  for (size_t i = 0; i < dict.size(); ++i) {
//
//    if (!dict.is_pruned(i)) {
//
//      if (!(*idx_pruned)[p_idx]) {
//        std::cout << dict.reference_words.at(i) << " ";
//      }
//      ++p_idx;
//    }
//  }
//  std::cout << std::endl;
//}

void writevsread() {
  // Quick speed test with optimizations disabled
  uint8_t x = 0;
                          // 5.80 baseline to calc idx + val
  std::bitset<5> bits(0); // 13.84 to write to bitset
                          // 11.62 to read from bitset
                          // 14.13 to write then read from bitset

  uint8_t arr[5];         // 5.90 to write to char array
                          // 5.99 to read from char arr
                          // 6.61 to write then read from char arr

  uint8_t idx, val;
  for (uint64_t i = 0; i < UINT_MAX / 5; ++i) {
    idx = i%5;
    val = i%2 - 1;
    bits[idx] = val;
    x = bits[idx];
    arr[idx] = val;
    x = arr[idx];
  }
}



void test_word() {
  std::string g("sissy");
  std::string s("essay");

  Word w1(g);
  Word w2(s);

  Guess old(g, s);
  std::cout << g << std::endl;
  std::cout << s << std::endl;
  std::cout << old << std::endl;
  std::cout << std::bitset<64>(old.id_string()) << std::endl;

  GuessPair guess_pair(w1, w2);
  std::cout << std::bitset<64>(guess_pair.id()) << std::endl;
}

void test_guess_construction(std::vector<std::string> wordlist) {
  std::vector<Word> words;
  for (std::string w : wordlist) {
    words.push_back(Word(w));
  }

  std::vector<GuessPair> guesses;

  for (size_t i = 0; i < words.size(); ++i) {
    for (size_t j = 0; j < words.size(); ++j) {
      const Word& g = words[i];
      const Word& s = words[j];
      //std::cout << g.get_word() << " " << s.get_word() << std::endl;
      GuessPair gp(g, s);
      Guess old(wordlist[i], wordlist[j]);
      assert(gp.id() == old.id_string());
      //guesses.push_back(GuessPair(g, s));
    }
  }

  std::cout << words.size() << std::endl;
  std::cout << guesses.size() << std::endl;
}

void test_gp_pindex(const std::vector<std::string>& wordlist) {
  GuessPairIndex gp_index(wordlist);
  std::cout << gp_index.size() << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "USAGE: ./wordle_bits wordlist [prune_index]" << std::endl;
    return 1;
  }

  std::vector<std::string> wordlist = load_wordlist(argv[1]);

  //for (int i = 0; i < 100; ++i) {
  //  std::cout << wordlist[i] << std::endl;
  //}

  //test_gp_pindex(wordlist);

  //prune_index_test(wordlist);
  //PruneIndex index = load_index(wordlist, argv[2]);
  //index_perf_test(wordlist);

  //PruneIndex pindex = argc == 3 ?
  //  PruneIndex(wordlist, argv[2]) :
  //  PruneIndex(wordlist);

  PruneIndex pindex(wordlist);
  std::ofstream file(argv[2]);
  pindex.save(file);
  //pindex._dump();
  file.close();

  PruneIndex ld(wordlist, argv[2]);
  //ld._dump();

  for (size_t i = 0; i < wordlist.size(); ++i) {
    for (size_t j = 0; j < wordlist.size(); ++j) {
      //std::cout << *pindex.prune(0, 0) << std::endl;
      //std::cout << *ld.prune(0, 0) << std::endl;
      assert(*pindex.prune(i,j) == *ld.prune(i,j));
    }
  }

  //index_helper_test(index);
  //test_word();
  //test_guess_construction(wordlist);

  exit(0);
}
