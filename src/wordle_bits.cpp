#include "dictionary.hpp"
#include "guess.hpp"
#include "guess_pair.hpp"
#include "guess_pair_index.hpp"
#include "prune_index.hpp"
#include "solver.hpp"
#include "word.hpp"
#include "wordle_solver.hpp"
#include "mean_wordle.hpp"

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

//double test_guess_index(const std::vector<std::string>& wordlist,
//                        const PruneIndex& index,
//                        const std::string& g,
//                        const boost::dynamic_bitset<>& pruned) {
//  boost::dynamic_bitset<> computed(pruned);
//
//  size_t acc = 0;
//  for (size_t i = 0; i < index.size(); ++i) {
//    if (computed[i]) {
//      continue;
//    }
//    std::string s = wordlist[i];
//
//    const boost::dynamic_bitset<>* s_pruned = index.prune(g, s);
//
//    computed |= ~*s_pruned;
//
//    size_t size = (~(*s_pruned | pruned)).count();
//    acc += size * size;
//  }
//
//  return (double) acc / (double) (~pruned).count();
//}
//
//void index_perf_test(const std::vector<std::string>& wordlist, const PruneIndex& index, const boost::dynamic_bitset<>& pruned) {
//  std::vector<std::pair<std::string, double>> average_sizes;
//
//  for (size_t i = 0; i < index.size(); ++i) {
//    std::string g = wordlist[i];
//    if (pruned[i]) {
//      continue;
//    }
//    double size = test_guess_index(wordlist, index, g, pruned);
//    average_sizes.push_back(std::pair<std::string, double>(g, size));
//  }
//
//  std::sort(average_sizes.begin(), average_sizes.end(),
//      [](const auto& a, const auto& b) {
//        return a.second < b.second;
//      });
//
//  //for (size_t i = 0; i < average_sizes.size(); ++i) {
//  for (size_t i = 0; i < average_sizes.size() && i < 15; ++i) {
//    auto& avg = average_sizes[i];
//    std::cout << avg.first << " " << avg.second << std::endl;
//  }
//  //return average_sizes;
//}

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "USAGE: ./wordle_bits wordlist [prune_index]" << std::endl;
    return 1;
  }

  std::vector<std::string> wordlist = load_wordlist(argv[1]);

  //std::cout << "Initializing prune index..." << std::endl;
  //PruneIndex tmp = argc == 3 ?
  //  PruneIndex(wordlist, argv[2]) :
  //  PruneIndex(wordlist);

  MeanWordle sol_only = argc == 3 ? MeanWordle(wordlist, argv[2]) :
                                    MeanWordle(wordlist);
  sol_only.play();

  //std::cout << "Solving" << std::endl;
  //WordleSolver solver(wordlist, std::move(tmp));

  //boost::dynamic_bitset<> pruned(wordlist.size());

  //bool done = false;
  //// 100: << 1s runtime, solution space = 6004
  //// 500: ~30s runtime, solution space =  822948
  //// 1000: 10m runtime, solution space =  8994353
  //while (!done) {
  //  if (pruned.count() == wordlist.size() - 1) {
  //    done = true;
  //  }
  //  std::pair<size_t, int> best = solver.solve(pruned);
  //  std::cout << best.first << " " << wordlist[best.first] << ": " << best.second << std::endl;

  //  pruned |= solver.make_guess(pruned, best.first);
  //  std::cout << pruned << std::endl;
  //  std::cout << std::endl;
  //}

}
