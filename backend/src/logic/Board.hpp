#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "DisjointSet.hpp"
#include "../dto/Chain.hpp"

class Board {
private:
   DisjointSet dsu;
   int cnt = 0;
   static constexpr int dirNum = 4;
   static constexpr std::array<int, dirNum> row = {0, -1, 1, 0};
   static constexpr std::array<int, dirNum> col = {-1, 0, 0, 1};  

   int encodeCoord(int x, int y) const {
      return (x - 1) * m + y;
   }

   std::pair<int, int> decodeCoord(int id) const {
      return {(id - 1) / m + 1, (id - 1) % m + 1};
   }

   bool isInBoard(int x, int y) const {
      return x >= 1 && y >= 1 && x <= n && y <= m;
   }

   int countLibertiesAround(int x, int y) {
      int cnt = 0;
      for (int k = 0; k < dirNum; ++k) {
         int u = x + row[k];
         int v = y + col[k];
         if (!isInBoard(u, v)) continue;
         if (dsu.isNodeEmpty(encodeCoord(u, v))) ++cnt;
      }
      return cnt;
   }

   int countLibertiesForRoot(int root) {
      std::unordered_set<int> liberties;
      for (int id = 1; id <= n * m; ++id) {
         if (dsu.isNodeEmpty(id)) continue;
         if (dsu.root(id) != root) continue;
         auto [x, y] = decodeCoord(id);
         for (int k = 0; k < dirNum; ++k) {
            int u = x + row[k];
            int v = y + col[k];
            if (!isInBoard(u, v)) continue;
            int nid = encodeCoord(u, v);
            if (dsu.isNodeEmpty(nid)) liberties.insert(nid);
         }
      }
      return static_cast<int>(liberties.size());
   }

   void joinChain(int x, int y, Color color) {
      int baseId = encodeCoord(x, y);
      for (int k = 0; k < dirNum; ++k) {
         int u = x + row[k];
         int v = y + col[k];
         if (!isInBoard(u, v)) continue;

         int nextId = encodeCoord(u, v);
         if (dsu.isNodeEmpty(nextId)) continue;
         if (dsu.getNodeColor(nextId) != color) continue;

         int rootA = dsu.root(baseId);
         int rootB = dsu.root(nextId);
         if (rootA == rootB) continue;

         int mergedRoot = dsu.join(rootA, rootB);
         int oldRoot = (mergedRoot == rootA) ? rootB : rootA;
         chains[mergedRoot].color = color;
         chains[mergedRoot].stoneCount = dsu.getSize(mergedRoot);
         chains[mergedRoot].libertyCount = countLibertiesForRoot(mergedRoot);
         chains[oldRoot] = Chain{};
      }
   }

   int capture(int x, int y, Color color) {
      std::unordered_set<int> capturedRoots;
      for (int k = 0; k < dirNum; ++k) {
         int u = x + row[k];
         int v = y + col[k];
         if (!isInBoard(u, v)) continue;

         int nextId = encodeCoord(u, v);
         if (dsu.isNodeEmpty(nextId)) continue;
         if (dsu.getNodeColor(nextId) == color) continue;

         int rootId = dsu.root(nextId);
         if (chains[rootId].libertyCount == 0) {
            capturedRoots.insert(rootId);
         }
      }

      int capturedCount = 0;
      for (int id = 1; id <= n * m; ++id) {
         if (dsu.isNodeEmpty(id)) continue;
         int rootId = dsu.root(id);

         if (capturedRoots.count(rootId)) {
            dsu.deleteNode(id);
            capturedCount++;
         }
      }
      cnt -= capturedCount;
      return capturedCount;
   }

   void rebuildChains() {
      chains.assign(n * m + 1, Chain{});
      std::unordered_set<int> roots;
      for (int id = 1; id <= n * m; ++id) {
         if (!dsu.isNodeEmpty(id)) {
            roots.insert(dsu.root(id));
         }
      }

      for (int rootId : roots) {
         chains[rootId] = Chain{
            dsu.getNodeColor(rootId),
            dsu.getSize(rootId),
            countLibertiesForRoot(rootId)
         };
      }
   }

public:
   int n;
   int m;
   std::vector<Chain> chains;

   Board(int n, int m) : dsu(n * m), cnt(0), n(n), m(m) {
      chains.assign(n * m + 1, Chain{});
   }

   bool isLegalMove(int x, int y) const {
      if (!isInBoard(x, y)) return false;
      int id = encodeCoord(x, y);
      return dsu.isNodeEmpty(id);
   }
   
   bool placeStone(int x, int y, Color color, std::string &error) {
      if (!isInBoard(x, y)) {
         error = "Move out of bounds.";
         return false;
      }
      int id = encodeCoord(x, y);
      if (!dsu.isNodeEmpty(id)) {
         error = "Point is already occupied.";
         return false;
      }

      dsu.initNode(id, color);
      chains[id] = Chain{color, 1, countLibertiesAround(x, y)};
      
      cnt++;
      joinChain(x, y, color);
      rebuildChains();
      capture(x, y, color);
      rebuildChains();
      if (chains[dsu.root(id)].libertyCount == 0) {
         dsu.deleteNode(id);
         cnt--;
         rebuildChains();
         error = "Suicide move is not allowed.";
         return false;
      }
      return true;
   }

   int countStone(Color color) const {
      int total = 0;
      for (int id = 1; id <= n * m; ++id) {
         if (!dsu.isNodeEmpty(id) && dsu.getNodeColor(id) == color) {
            total++;
         }
      }
      return total;
   }

   bool isBoardFull() const {
      return cnt == n * m;
   }

   void reset() {
      dsu = DisjointSet(n * m);
      chains.assign(n * m + 1, Chain{});
      cnt = 0;
   }

   std::vector<Color> getBoardColors() const {
      std::vector<Color> out(n * m, Color::Empty);
      for (int id = 1; id <= n * m; ++id) {
         if (!dsu.isNodeEmpty(id)) {
            out[id - 1] = dsu.getNodeColor(id);
         }
      }
      return out;
   }

   std::vector<int> getRootMap() const {
      std::vector<int> out(n * m, -1);
      for (int id = 1; id <= n * m; ++id) {
         if (!dsu.isNodeEmpty(id)) {
            out[id - 1] = dsu.root(id) - 1;
         }
      }
      return out;
   }
};
