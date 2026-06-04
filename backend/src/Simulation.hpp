#pragma once

#include <vector>
#include <utility>

#include "./logic/Board.hpp"
#include "./strategy/RandomStrategy.hpp"
#include "Player.hpp"

enum class GameState { Running, Finished };

class Simulation {
private:
   Player &player1;
   Player &player2;
   Board board;

   bool turn;
   Player *currentPlayer;
   GameState state = GameState::Running;
   std::vector<std::pair<int, int>> history;

public:
   Simulation(Player &player1, Player &player2, Board board)
      : player1(player1), player2(player2), board(board) {}
   
   void initSimulation(int n, int m) {
      board = Board{n, m};
      player1.color = Color::White;
      player1.strategy = std::make_unique<RandomStrategy>();
      player2.color = Color::Black;
      player2.strategy = std::make_unique<RandomStrategy>();
      currentPlayer = &player1;
      turn = 0;
      state = GameState::Running;
      history.clear();
   }

   void reset() {
      board.reset();
      currentPlayer = &player1;
      turn = 0;
      state = GameState::Running;
      history.clear();
   }

   void runSimulation() {
      while (!board.isBoardFull()) {
         giveTurn(*currentPlayer);
         if (!turn) {
            currentPlayer = &player2;
            turn = 1;
         } else {
            currentPlayer = &player1;
            turn = 0;
         }
      }
      state = GameState::Finished;
   }

   void giveTurn(Player &player) {
      auto [x, y] = player.strategy->chooseMove(board, player.color);
      std::string error;
      history.push_back({x, y});
      board.placeStone(x, y, player.color, error);
   }

   const Board &getBoard() const {
      return board;
   }

   const std::vector<std::pair<int, int>> &getHistory() const {
      return history;
   }
};
