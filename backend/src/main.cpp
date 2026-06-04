#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Simulation.hpp"
#include "Player.hpp"
#include "strategy/RandomStrategy.hpp"
#include "logic/Board.hpp"
#include "dto/Color.hpp"

std::string quoteJson(const std::string &value) {
   std::string result = "\"";
   for (char ch : value) {
      switch (ch) {
         case '\\': result += "\\\\"; break;
         case '"': result += "\\\""; break;
         case '\n': result += "\\n"; break;
         case '\r': result += "\\r"; break;
         case '\t': result += "\\t"; break;
         default: result += ch; break;
      }
   }
   result += '"';
   return result;
}

std::string colorToString(Color color) {
   switch (color) {
      case Color::Black: return "Black";
      case Color::White: return "White";
      default: return "Empty";
   }
}

Color stringToColor(const std::string &value) {
   if (value == "Black") return Color::Black;
   if (value == "White") return Color::White;
   return Color::Empty;
}

std::vector<std::string> splitArgs(const std::string &line) {
   std::vector<std::string> args;
   std::istringstream iss(line);
   std::string token;
   while (iss >> token) {
      args.push_back(token);
   }
   return args;
}

std::string serializeState(const Board &board) {
   std::ostringstream oss;
   const auto colors = board.getBoardColors();
   const auto roots = board.getRootMap();
   const auto chains = board.chains;
   int size = board.n;

   oss << "{";
   oss << "\"size\": " << size << ", ";

   oss << "\"board\": [";
   for (size_t i = 0; i < colors.size(); ++i) {
      if (i) oss << ", ";
      oss << quoteJson(colorToString(colors[i]));
   }
   oss << "], ";

   oss << "\"root_map\": [";
   for (size_t i = 0; i < roots.size(); ++i) {
      if (i) oss << ", ";
      oss << roots[i];
   }
   oss << "], ";

   oss << "\"chains\": {";
   bool first = true;
   for (size_t i = 0; i < chains.size(); ++i) {
      if (chains[i].color == Color::Empty) continue;
      if (!first) oss << ", ";
      first = false;
      oss << quoteJson(std::to_string(i - 1)) << ": {";
      oss << "\"color\": " << quoteJson(colorToString(chains[i].color)) << ", ";
      oss << "\"stones\": " << chains[i].stoneCount << ", ";
      oss << "\"liberties\": " << chains[i].libertyCount;
      oss << "}";
   }
   oss << "}";

   oss << "}";
   return oss.str();
}

std::string serializeHistory(const std::vector<std::pair<int, int>> &history) {
   std::ostringstream oss;
   oss << "[";
   for (size_t i = 0; i < history.size(); ++i) {
      if (i) oss << ", ";
      auto [x, y] = history[i];
      Color color = (i % 2 == 0) ? Color::White : Color::Black;
      oss << "{";
      oss << "\"x\": " << x << ", ";
      oss << "\"y\": " << y << ", ";
      oss << "\"color\": " << quoteJson(colorToString(color));
      oss << "}";
   }
   oss << "]";
   return oss.str();
}

int main() {
   Board board(9, 9);
   std::string line;

   while (std::getline(std::cin, line)) {
      if (line.empty()) continue;
      const auto args = splitArgs(line);
      if (args.empty()) continue;

      const std::string cmd = args[0];
      if (cmd == "EXIT") {
         std::cout << "OK" << std::endl;
         break;
      }

      if (cmd == "RESET") {
         if (args.size() == 3) {
            int n = std::stoi(args[1]);
            int m = std::stoi(args[2]);
            board = Board(n, m);
         } else {
            board.reset();
         }
         std::cout << "OK " << serializeState(board) << std::endl;
         continue;
      }

      if (cmd == "STATE") {
         std::cout << "OK " << serializeState(board) << std::endl;
         continue;
      }

      if (cmd == "PLAY") {
         if (args.size() != 4) {
            std::cout << "ERROR invalid play command" << std::endl;
            continue;
         }
         int x = std::stoi(args[1]);
         int y = std::stoi(args[2]);
         Color color = stringToColor(args[3]);
         std::string error;
         if (!board.placeStone(x, y, color, error)) {
            std::cout << "ERROR " << quoteJson(error) << std::endl;
            continue;
         }
         std::cout << "OK " << serializeState(board) << std::endl;
         continue;
      }

      if (cmd == "AI_MOVE") {
         if (args.size() != 2) {
            std::cout << "ERROR invalid ai_move command" << std::endl;
            continue;
         }
         Color color = stringToColor(args[1]);
         RandomStrategy strategy;
         auto [x, y] = strategy.chooseMove(board, color);
         if (x < 1 || y < 1) {
            std::cout << "OK {\"move\": null, \"state\": " << serializeState(board) << "}" << std::endl;
            continue;
         }
         std::string error;
         if (!board.placeStone(x, y, color, error)) {
            std::cout << "ERROR " << quoteJson(error) << std::endl;
            continue;
         }
         std::cout << "OK {\"move\": {\"x\": " << x << ", \"y\": " << y << ", \"color\": " << quoteJson(colorToString(color)) << "}, \"state\": " << serializeState(board) << "}" << std::endl;
         continue;
      }

      if (cmd == "SIMULATE") {
         if (args.size() != 3) {
            std::cout << "ERROR invalid simulate command" << std::endl;
            continue;
         }
         int n = std::stoi(args[1]);
         int m = std::stoi(args[2]);
         Player player1(Color::White, std::make_unique<RandomStrategy>());
         Player player2(Color::Black, std::make_unique<RandomStrategy>());
         Simulation sim(player1, player2, Board(n, m));
         sim.initSimulation(n, m);
         sim.runSimulation();
         std::string stateJson = serializeState(sim.getBoard());
         if (!stateJson.empty() && stateJson.back() == '}') {
            stateJson.pop_back();
         }
         std::cout << "OK " << stateJson << ", \"history\": " << serializeHistory(sim.getHistory()) << "}" << std::endl;
         continue;
      }

      std::cout << "ERROR unknown command" << std::endl;
   }

   return 0;
}
