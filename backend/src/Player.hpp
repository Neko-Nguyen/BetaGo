#pragma once

#include <memory>

#include "./dto/Color.hpp"
#include "./logic/Board.hpp"
#include "./strategy/PlayerStrategy.hpp"

class Player {
public:
   Color color;
   std::unique_ptr<PlayerStrategy> strategy;

   Player(Color color, std::unique_ptr<PlayerStrategy> strat)
      : color(color), strategy(std::move(strat)) {}
};