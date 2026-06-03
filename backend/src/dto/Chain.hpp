#pragma once

#include "Color.hpp"

struct Chain {
   Color color = Color::Empty;
   int stoneCount = 0;
   int libertyCount = 0;
};