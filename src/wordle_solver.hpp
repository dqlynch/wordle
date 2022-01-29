#ifndef WORDLE_SOLVER_H
#define WORDLE_SOLVER_H

#include "constants.hpp"
#include "guess_pair.hpp"
#include "prune_index.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>


class WordleSolver {
 public:
  WordleSolver(std::vector<std::string> wordlist)
    : size_(wordlist.size()), pindex_(PruneIndex(wordlist)) {}

  WordleSolver(PruneIndex&& pindex)
    : size_(pindex.size()), pindex_(std::move(pindex)) {}

  /**
   * Player picks the best guess that minimizes his path.
   * Antagonist picks the solution for the given guess that maximizes the path.
   * Both return a pair<idx, path_length> where word[idx] is the word.
   */
  std::pair<size_t, int> player(const boost::dynamic_bitset<>& pruned, int depth);
  std::pair<size_t, int> antagonist(boost::dynamic_bitset<> pruned,
                                    size_t g_idx, int depth);
  std::pair<size_t, int> solve(boost::dynamic_bitset<> pruned) {
    assert(pruned.size() == size_);
    auto ans = player(pruned, 0);
    std::cout << "Memo size: " << memo_.size() << std::endl;
    return ans;
  }

  std::pair<size_t, int> solve() {
    return solve(boost::dynamic_bitset<>(size_));
  }

  std::pair<size_t, boost::dynamic_bitset<>> make_guess(boost::dynamic_bitset<> pruned, size_t g_idx);

 private:
   static bool cmp(std::pair<size_t, int> a, std::pair<size_t, int> b) {
     return a.second < b.second;
   }

  std::unordered_map<boost::dynamic_bitset<>, std::pair<int, size_t>> memo_;

  size_t size_;
  const PruneIndex pindex_;
};

std::pair<size_t, int> WordleSolver::player(const boost::dynamic_bitset<>& pruned, int depth) {
  if (memo_.count(pruned)) {
    return memo_.at(pruned);
  }

  if (pruned.count() == 1) {
    // There's only one solution, we always guess it.
    return std::pair<size_t, int>(0, 1);
  }

  // TODO base cases where all pruned or only one

  std::pair<size_t, int> best_guess(0, INT_MAX);

  for (size_t g_idx = 0; g_idx < size_; ++g_idx) {
    if (pruned[g_idx]) {
      continue;
    }
    if (depth == 0) {
      //std::cout << g_idx << ": " << wordlist_[g_idx] << std::endl;
    }

    std::pair<size_t, int> guess(g_idx, antagonist(pruned, g_idx, depth).second);

    best_guess = std::min(best_guess, guess, cmp);
  }

  memo_.insert({pruned, best_guess});

  return best_guess;
}

std::pair<size_t, int> WordleSolver::antagonist(boost::dynamic_bitset<> pruned,
                                                size_t g_idx, int depth) {
  std::pair<size_t, int> worst_solution(0, 0);
  boost::dynamic_bitset<> computed(size_);

  for (size_t s_idx = 0; s_idx < size_; ++s_idx) {
    if (pruned[s_idx] || computed[s_idx]) {
      assert(s_idx != g_idx);
      continue;
    }

    if (g_idx == s_idx) {
      // Player guessed the right word
      worst_solution = std::max(worst_solution, std::pair<size_t, int>(s_idx, 1), cmp);
      continue;
    }

    // TODO computed guesses
    const boost::dynamic_bitset<>* gs_pruned = pindex_.prune(g_idx, s_idx);
    computed |= ~*gs_pruned;
    boost::dynamic_bitset<> next_pruned = pruned | *gs_pruned;

    std::pair<size_t, int> solution(s_idx, player(next_pruned, depth + 1).second + 1);
    //std::cout << "Considering " << wordlist_[solution.first] << ": " << solution.second << std::endl;

    worst_solution = std::max(worst_solution, solution, cmp);
  }

  return worst_solution;
}

std::pair<size_t, boost::dynamic_bitset<>> WordleSolver::make_guess(boost::dynamic_bitset<> pruned, size_t g_idx) {
  std::pair<size_t, int> worst_solution = antagonist(pruned, g_idx, 0);
  std::cout << "Best possible: " << worst_solution.second << std::endl;
  return std::pair<size_t, boost::dynamic_bitset<>>(worst_solution.first,
      pruned | *pindex_.prune(g_idx, worst_solution.first));
}

#endif
