#pragma once

#include <algorithm>
#include <vector>

#include "../dto/Color.hpp"

class DisjointSet {
private:
   struct Node {
      Color color = Color::Empty;
      int parent = 0;
      int size = 0;
   };

   int size;
   std::vector<Node> nodes;
public:
   DisjointSet(int size): size(size) {
      nodes.resize(size + 1);
   }

   bool isNodeEmpty(int id) const {
      return nodes[id].parent == 0;
   }

   void initNode(int id, Color color) {
      nodes[id].parent = id;
      nodes[id].size = 1;
      nodes[id].color = color;
   }

   void deleteNode(int id) {
      nodes[id].parent = nodes[id].size = 0;
      nodes[id].color = Color::Empty;
   }

   Color getNodeColor(int id) const {
      return nodes[id].color;
   }

   bool isInSameChain(int id1, int id2) const {
      id1 = root(id1);
      id2 = root(id2);
      return id1 == id2;
   }
   
   int root(int id) {
      return nodes[id].parent == id ? id : nodes[id].parent = root(nodes[id].parent);
   }

   int root(int id) const {
      return nodes[id].parent == id ? id : root(nodes[id].parent);
   }

   int getSize(int id) const {
      id = root(id);
      return nodes[id].size;
   }

   int join(int id1, int id2) {
      id1 = root(id1);
      id2 = root(id2);
      if (id1 == id2) return id1;
      if (nodes[id1].size < nodes[id2].size) std::swap(id1, id2);
      nodes[id1].size += nodes[id2].size;
      nodes[id2].parent = id1;
      return id1;
   }
};
