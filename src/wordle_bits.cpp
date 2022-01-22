#include "dictionary.hpp"
#include "guess.hpp"
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

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: ./wordle_bits wordlist" << std::endl;
    return 1;
  }

  std::vector<std::string> wordlist = load_wordlist(argv[1]);
  Dictionary dict(wordlist);

  //auto avgs = perf_test(wordlist);
  //for (size_t i = 0; i < avgs.size(); ++i) {
  //  std::cout << avgs[i].first << std::endl;
  //}
  //return 0;

  // Test solve on a prepruned set to start
  //Guess guess("jujus", "fluff");
  //Guess guess("jujus", "share");
  Guess guess("raise", "aural");

  dict.prune(guess);
  std::cout << guess << std::endl;
  std::cout << "Dict size: " << dict.count() << std::endl;

  //Guess next("steed")

  //exit(2);

  Solver solver(&dict);
  auto best = solver.solve();

  std::cout << best.first << " " << best.second << std::endl;
  solver.print_remaining(std::cout);

  //std::cout << solver.make_guess("stray") << std::endl;
  //best = solver.player();
  //std::cout << best.first << " " << best.second << std::endl;
  //solver.print_remaining(std::cout);

  return 0;
}
