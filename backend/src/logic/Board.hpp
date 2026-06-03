#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "DisjointSet.hpp"
#include "../dto/Chain.hpp"

class Board {
private:
   int n;
   int m;
   DisjointSet dsu;
   std::vector<Chain> chains;

   const int dirNum = 4;
   const std::vector<int> row = {0, -1, 1, 0};
   const std::vector<int> col = {-1, 0, 0, 1};

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

   void recomputeChains() {
      for (int id = 1; id <= n * m; ++id) {
         chains[id] = Chain{};
      }

      std::vector<std::unordered_set<int>> libertySets(n * m + 1);
      for (int id = 1; id <= n * m; ++id) {
         if (dsu.isNodeEmpty(id)) continue;
         int rootId = dsu.root(id);
         if (chains[rootId].color == Color::Empty) {
            chains[rootId].color = dsu.getNodeColor(id);
         }
         chains[rootId].stoneCount += 1;
         auto [x, y] = decodeCoord(id);
         for (int k = 0; k < dirNum; ++k) {
            int u = x + row[k];
            int v = y + col[k];
            if (!isInBoard(u, v)) continue;
            int nid = encodeCoord(u, v);
            if (dsu.isNodeEmpty(nid)) {
               libertySets[rootId].insert(nid);
            }
         }
      }

      for (int id = 1; id <= n * m; ++id) {
         if (chains[id].color != Color::Empty) {
            chains[id].libertyCount = static_cast<int>(libertySets[id].size());
         }
      }
   }

public:
   Board(int n, int m) : n(n), m(m), dsu(n * m) {
      chains.resize(n * m + 1);
   }

   bool isLegalMove(int x, int y, [[maybe_unused]] Color color) {
      if (!isInBoard(x, y)) return false;
      int id = encodeCoord(x, y);
      return dsu.isNodeEmpty(id);
   }

   void placeStone(int x, int y, Color color) {
      int id = encodeCoord(x, y);
      dsu.initNode(id, color);
      chains[id] = Chain{color, 1, countLibertiesAround(x, y)};

      joinChain(x, y, color);
      capture(x, y, color);
      recomputeChains();
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
            chains[id] = Chain{};
            capturedCount++;
         }
      }

      return capturedCount;
   }
};
