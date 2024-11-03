#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace mapf {
struct Edge {
  int fromX = 0;
  int fromY = 0;
  int toX = 0;
  int toY = 0;
};

struct Map {
  std::string type = "";
  int width = 0;
  int height = 0;
  std::vector<int> vertices;

  int get_vertex(size_t x, size_t y) const;
};

struct GraphNode {
  int value = 0;
  int x = 0;
  int y = 0;
  std::vector<GraphNode *> edges;
};

struct Graph {
  std::vector<GraphNode *> nodes;

  GraphNode *get(int x, int y);
};

Map loadMap(std::string path);

Graph getGraph(Map map);
} // namespace mapf