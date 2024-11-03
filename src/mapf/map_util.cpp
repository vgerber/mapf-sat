#include "map_util.h"

namespace mapf {

GraphNode *Graph::get(int x, int y) {
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i]->x == x && nodes[i]->y == y) {
      return nodes[i];
    }
  }
  return nullptr;
}

int Map::get_vertex(size_t x, size_t y) const {
  if (x >= width || y >= height) {
    return -1;
  }
  return vertices[x * width + y];
}

Map loadMap(std::string path) {
  std::fstream mapFile(path, std::fstream::in);

  Map outputMap;

  if (mapFile.good()) {
    std::string line;
    bool mapStart = false;
    int row = 0;
    while (std::getline(mapFile, line)) {
      if (!mapStart) {
        for (int i = 0; i < line.size(); i++) {
          if (line[i] == ' ') {
            std::string type = line.substr(0, i);
            std::string value = line.substr(i + 1, line.size() - (i + 1));
            if (type == "type") {
              outputMap.type = value;
            } else if (type == "height") {
              outputMap.height = std::stoi(value);
            } else if (type == "width") {
              outputMap.width = std::stoi(value);
            }
          } else if (line == "map") {
            outputMap.vertices =
                std::vector<int>(outputMap.width * outputMap.height);
            mapStart = true;
          }
        }
      } else {
        for (int i = 0; i < line.size(); i++) {
          int value = 0;
          switch (line[i]) {
          case 'T':
            value = 1;
            break;
          case '.':
            value = 0;
            break;
          case '@':
            value = -1;
            break;
          default:
            break;
          }

          outputMap.vertices[outputMap.width * row + i] = value;
        }
        row++;
      }
    }
  } else {
    printf("Can't open file %s\n", path.c_str());
  }
  return outputMap;
}

Graph getGraph(Map map) {
  const unsigned int dMax = 04;
  int dxKernel[dMax] = {0, 1, 0, -1};
  int dyKernel[dMax] = {1, 0, -1, 0};

  std::vector<GraphNode *> nodes;

  for (int y = 0; y < map.width; y++) {
    for (int x = 0; x < map.height; x++) {
      if (map.vertices[y * map.width + x] == 0) {
        GraphNode *node = new GraphNode();
        node->value = 0;
        node->x = x;
        node->y = y;
        for (int k = std::max(y - 1 * map.width, 0); k < nodes.size(); k++) {
          GraphNode *graphNode = nodes[k];
          if (abs(graphNode->x - x) == 1 && abs(graphNode->y - y) == 0) {
            node->edges.push_back(graphNode);
            graphNode->edges.push_back(node);
          } else if (abs(graphNode->y - y) == 1 && abs(graphNode->x - x) == 0) {
            node->edges.push_back(graphNode);
            graphNode->edges.push_back(node);
          }
        }
        nodes.push_back(node);
      }
    }
  }
  Graph graph;
  graph.nodes = nodes;
  return graph;
}
} // namespace mapf