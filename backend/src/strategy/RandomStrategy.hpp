#pragma once

#include <random>
#include <vector>

#include "PlayerStrategy.hpp"

struct RandomStrategy: public PlayerStrategy {
private:
   std::mt19937 rng{std::random_device{}()};

public:
   std::pair<int, int> chooseMove(const Board &board, Color color) override {
      std::vector<std::pair<int, int>> candidates;
      candidates.reserve(board.n * board.m);

      for (int x = 1; x <= board.n; ++x) {
         for (int y = 1; y <= board.m; ++y) {
            if (board.isLegalMove(x, y)) {
               candidates.emplace_back(x, y);
            }
         }
      }

      if (candidates.empty()) {
         return {-1, -1};
      }

      std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
      return candidates[dist(rng)];
   }  
};