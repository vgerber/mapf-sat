#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace mapf {
typedef int CoordinateT;

struct Edge {
  CoordinateT from_x = 0;
  CoordinateT from_y = 0;
  CoordinateT to_x = 0;
  CoordinateT to_y = 0;
};

struct Map {
  std::string type = "";
  CoordinateT width = 0;
  CoordinateT height = 0;
  std::vector<int> vertices;

  int get_vertex(CoordinateT x, CoordinateT y) const;
};
typedef std::shared_ptr<Map> MapPtr;

struct GraphNode {
  int value = 0;
  CoordinateT x = 0;
  CoordinateT y = 0;
  std::vector<std::shared_ptr<GraphNode>> edges;
};
typedef std::shared_ptr<GraphNode> GraphNodePtr;

struct Graph {
  CoordinateT width = 0;
  CoordinateT height = 0;
  std::vector<GraphNodePtr> nodes;

  std::shared_ptr<GraphNode> get_node(CoordinateT x, CoordinateT y);
};
typedef std::shared_ptr<Graph> GraphPtr;

MapPtr load_map(const std::filesystem::path &path);

GraphPtr map_to_graph(const MapPtr map);

CoordinateT get_distance(CoordinateT a, CoordinateT b);
} // namespace mapf