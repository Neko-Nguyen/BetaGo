#pragma once

#include <utility>

#include "../logic/Board.hpp"
#include "../dto/Color.hpp"

struct PlayerStrategy {
public:
   virtual std::pair<int, int> chooseMove(const Board &board, Color color) = 0;
   virtual ~PlayerStrategy() = default;
};