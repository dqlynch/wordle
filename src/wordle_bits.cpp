#include "dictionary.hpp"
#include "guess.hpp"
#include "prune_index.hpp"
#include "solver.hpp"

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

double test_guess_index(const PruneIndex& index, std::string g) {
  boost::dynamic_bitset<> computed(index.size(), 0);

  size_t acc = 0;
  for (size_t i = 0; i < index.size(); ++i) {
    if (computed[i]) {
      continue;
    }
    std::string s = index.wordlist()[i];
    Guess guess(g, s);

    const boost::dynamic_bitset<>* s_pruned = index.prune(g, s);

    computed |= ~*s_pruned;


    size_t size = count_zeros(*s_pruned);
    acc += size * size;
  }

  return (double) acc / (double) index.size();
}

void index_perf_test(const std::vector<std::string>& wordlist, std::string filename) {
  std::cout << "Loading file..." << std::endl;
  std::ifstream file(filename);
  PruneIndex index(&wordlist, file);
  std::cout << "Testing guesses..." << std::endl;

  std::vector<std::pair<std::string, double>> average_sizes;

  for (std::string g : wordlist) {
    double size = test_guess_index(index, g);
    average_sizes.push_back(std::pair<std::string, double>(g, size));
  }

  std::sort(average_sizes.begin(), average_sizes.end(),
      [](const auto& a, const auto& b) {
        return a.second < b.second;
      });

  for (size_t i = 0; i < average_sizes.size(); ++i) {
    auto& avg = average_sizes[i];
    std::cout << avg.first << " " << avg.second << std::endl;
  }
  //return average_sizes;
}

void prune_index_test(const std::vector<std::string>& wordlist) {
  // Initially prune for a smaller test set
  Dictionary dict(wordlist);
  //Guess guess("raise", "aural");   // --> 78 words
  //Guess guess("jujus", "share");    // >> 466 words
  //dict.prune(guess);

  std::vector<std::string> pruned_wordlist;
  for (size_t i = 0; i < wordlist.size(); ++i) {
    if (!dict.is_pruned(i)) {
      pruned_wordlist.push_back(wordlist.at(i));
    }
  }

  //PruneIndex save_idx(&wordlist);
  //PruneIndex index(&wordlist);

  // Save test
  //std::ofstream of("solution_words.pindex");
  //save_idx.save(of);
  //exit(0);

  std::cout << "loading..." << std::endl;
  std::ifstream file("solution_words.pindex");
  PruneIndex index(&pruned_wordlist, file);
  std::cout << "loaded..." << std::endl;

  std::cout << index.num_guesses_ << std::endl;
  std::cout << index.num_prune_checks_ << std::endl;

  // Check traditionally
  Guess cmp("raise", "torso");
  dict.prune(cmp);

  for (size_t i = 0; i < dict.size(); ++i) {
    if (!dict.is_pruned(i)) {
      std::cout << dict.reference_words.at(i) << " ";
    }
  }
  std::cout << std::endl;
  std::cout << dict.count() << std::endl;

  dict.pop();

  // Check with PruneIndex
  const boost::dynamic_bitset<>* idx_pruned = index.prune(cmp);
  std::cout << *idx_pruned << std::endl;

  size_t p_idx = 0;
  for (size_t i = 0; i < dict.size(); ++i) {

    if (!dict.is_pruned(i)) {

      if (!(*idx_pruned)[p_idx]) {
        std::cout << dict.reference_words.at(i) << " ";
      }
      ++p_idx;
    }
  }
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "USAGE: ./wordle_bits wordlist [prune_index]" << std::endl;
    return 1;
  }

  std::vector<std::string> wordlist = load_wordlist(argv[1]);


  //prune_index_test(wordlist);
  index_perf_test(wordlist, argv[2]);

  Dictionary dict(wordlist);

  //auto avgs = perf_test(wordlist);
  //for (size_t i = 2310; i < avgs.size(); ++i) {
  //  std::cout << avgs[i].first << " " << avgs[i].second << std::endl;
  //}
  //return 0;

  //// Test solve on a prepruned set to start
  //Guess guess("raise", "aural");    // 78:  0.64 -> 4 dwarf
  ////Guess guess("jujus", "fluff");    // 134: 3.76 -> 4 thumb
  ////Guess guess("jujus", "share");    // 466: ?

  //dict.prune(guess);
  //std::cout << guess << std::endl;
  //std::cout << "Dict size: " << dict.count() << std::endl;

  ////Guess next("steed")

  ////exit(2);

  //Solver solver(&dict);
  //auto best = solver.solve();

  //std::cout << best.first << " " << best.second << std::endl;
  //solver.print_remaining(std::cout);

  ////std::cout << solver.make_guess("stray") << std::endl;
  ////best = solver.player();
  ////std::cout << best.first << " " << best.second << std::endl;
  ////solver.print_remaining(std::cout);

  //return 0;
}
