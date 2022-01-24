#ifndef SOLVER_H
#define SOLVER_H

#include "constants.hpp"
#include "dictionary.hpp"
#include "guess.hpp"

#include <limits.h>

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

// We want to use an arbitrarily large number as bound without getting close to
// UINT_MAX, as we increment returned max bounds and don't want to overflow.
const size_t MAX_VALUE = INT_MAX;

class Solver {
 public:
   Solver(Dictionary* dictionary)
   : dictionary_(dictionary){}

   ~Solver() {
     for (auto& [k, v] : computed_guesses_) {
       delete v;
     }
   }

   /**
    * Determine the optimal guess given an optimally antagonistic game.
    *
    * Bound is an upper bound on the optimal chain length of the best guess.
    */
   std::pair<unsigned int, std::string> player(unsigned int bound);

   std::pair<unsigned int, std::string> solve() {
     auto val = player(MAX_VALUE);
     std::cout << "num prunes: " << num_prunes << std::endl;
     std::cout << "memo_ hits:   " << memo_hits_ << std::endl;
     std::cout << "memo_ misses: " << memo_misses_ << std::endl;
     std::cout << "guess hits:   " << ghits_ << std::endl;
     std::cout << "guess misses: " << gmisses_ << std::endl;
     return val;
   }

   /**
    * Determine the antagonistically optimal solution given a guess g which
    * maximizes the chain length assuming optimal play.
    */
   std::pair<unsigned int, std::string> antagonist(std::string g, unsigned int bound);

   const Guess make_guess(std::string g);

   void print_remaining(std::ostream& os);

 private:
   std::unordered_map<std::vector<bool>, std::pair<unsigned int, std::string>> memo_;
   std::unordered_map<std::string, Guess*> computed_guesses_;   // Save computed guesses

   static bool compare(std::pair<unsigned int, std::string> a, std::pair<unsigned int, std::string> b) {
     return a.first < b.first;
   }

   Dictionary* const dictionary_;

   size_t depth_ = 0;

   size_t num_prunes = 0;

   size_t memo_misses_ = 0;
   size_t memo_hits_ = 0;
   size_t ghits_ = 0;
   size_t gmisses_ = 0;
};

/**
 * Public implementations
 */
std::pair<unsigned int, std::string> Solver::player(unsigned int bound) {
  // Fast exit: Only one word to guess, we solve on this guess.
  if (dictionary_->count() == 1) {
    for (size_t i = 0; i < dictionary_->size(); ++i) {
      if (!dictionary_->is_pruned(i)) {
        return std::pair<unsigned int, std::string>(1, dictionary_->reference_words.at(i));
      }
    }
  }
  if (bound == 1) {
    // TODO This is not currently working (sometimes returns different results
    // when using bound pruning) and I'm not sure why.

    // We know we cannot find a guess better or equal to 1 (see fast exit above),
    // so we cannot beat bound in this recursion. Return a value greater than bound
    // with a dummy word value.
    //num_prunes++;
    //return std::pair<unsigned int, std::string>(2, "PRUNED");
  }


  auto key = dictionary_->key();
  if (memo_.count(key)) {
    ++memo_hits_;
    return memo_.at(key);
  }

  // <optimal solve length, guess>
  std::pair<unsigned int, std::string> best_worst_case(MAX_VALUE, "");

  // Pick best word out of unpruned words
  for (size_t i = 0; i < dictionary_->size(); ++i) {
    if (dictionary_->is_pruned(i)) {
      continue;
    }

    std::string g = dictionary_->reference_words.at(i);

    ++depth_;
    std::pair<unsigned int, std::string> worst_case = antagonist(g, bound);
    --depth_;

    if (depth_ == 0) {
      std::string gap;
      for (size_t i = 0; i < depth_; ++i) {
        gap += "--";
      }
      std::cout << gap << std::endl;
      std::cout << gap << "p" << i << "\n";
      std::cout << gap << g << ":" << worst_case.first << " (" << worst_case.second << ")" <<"\n";
      worst_case.second = g;
      best_worst_case = std::min(best_worst_case, worst_case, compare);
      bound = std::min(bound, best_worst_case.first);
      std::cout << gap << best_worst_case.second << ": " << best_worst_case.first << " b:"<< bound << "\n";
      std::cout << gap << std::endl;
    }

    worst_case.second = g;
    best_worst_case = std::min(best_worst_case, worst_case, compare);
    bound = std::min(bound, best_worst_case.first);
  }

  memo_.insert({key, best_worst_case});
  ++memo_misses_;

  assert(best_worst_case.first > 1);

  return best_worst_case;
}

std::pair<unsigned int, std::string> Solver::antagonist(std::string g, unsigned int bound) {
  unsigned int path_sum = 0;   // TODO
  std::pair<unsigned int, std::string> longest_solve(0, "");

  std::vector<bool> computed(dictionary_->reference_words.size(), 0);
  for (size_t i = 0; i < dictionary_->size(); ++i) {
    if (dictionary_->is_pruned(i) || computed[i]) {
      continue;
    }
    std::string s = dictionary_->reference_words.at(i);

    if (g == s) {
      longest_solve = std::max(longest_solve, std::pair<unsigned int, std::string>(1, s), compare);
      ++path_sum;
      continue;
    }

    Guess* guess;
    std::string gkey = g+s;
    if (computed_guesses_.count(gkey)) {
      ++ghits_;
      guess = computed_guesses_.at(gkey);
    } else {
      ++gmisses_;
      guess = new Guess(g, s);
      computed_guesses_.insert({gkey, guess});
    }

    std::vector<bool>* pruned = dictionary_->prune(*guess);

    // Use insight that the set this guess reduces to == the set of guesses
    // that dedupe with this guess to skip duplicate guess computations
    for (size_t j = 0; j < pruned->size(); ++j) {
      if (!pruned->at(j)) {
        computed[j] = 1;
      }
    }

    // Player's best solve given this g-s pair
    std::pair<unsigned int, std::string> solve = player(bound - 1);
    ++solve.first;
    solve.second = s;

    // This g-s is weighted by the set size it reduces to
    path_sum += dictionary_->count() * solve.first;

    dictionary_->pop(); // Reset pruned_ to starting state

    //if (depth_ == 1) {
    //if (g == "steed" && depth_ == 1) {
    //  std::cout << "a" << i << std::endl;

    //  std::cout << *guess << " : " << s << std::endl;
    //  std::cout << "this: " << solve.first << " " << solve.second << std::endl;
    //  std::cout << "best: " << longest_solve.first << " " << longest_solve.second << std::endl;
    //  std::cout << "count: " << dictionary_->count() << std::endl;
    //}

    longest_solve = std::max(longest_solve, solve, compare);
  }

  longest_solve.first = path_sum;
  return longest_solve;
}

const Guess Solver::make_guess(std::string g) {
  auto worst_case = antagonist(g, MAX_VALUE);
  std::cout << worst_case.first << " " << worst_case.second << std::endl;
  Guess guess(g, worst_case.second);

  dictionary_->prune(guess);

  return guess;
}

void Solver::print_remaining(std::ostream& os) {
  os << "{ ";
  for (size_t i = 0; i < dictionary_->size(); ++i) {
    if (!dictionary_->is_pruned(i)) {
      os << dictionary_->reference_words.at(i) << " ";
    }
  }
  os << "}" << std::endl;
}

#endif
