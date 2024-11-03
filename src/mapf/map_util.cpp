#include "map_util.h"

namespace mapf {

GraphNodePtr Graph::get_node(CoordinateT x, CoordinateT y) {
  if (x >= width || y >= height || x < 0 || y < 0) {
    return nullptr;
  }
  return nodes[y * width + x];
}

int Map::get_vertex(CoordinateT x, CoordinateT y) const {
  if (x >= width || y >= height) {
    return -1;
  }
  return vertices[x * width + y];
}

void parse_map_header_line(const MapPtr output_map,
                           const std::string &file_line,
                           bool &is_map_start_line) {
  if (file_line == "map") {
    output_map->vertices =
        std::vector<int>(output_map->width * output_map->height);
    is_map_start_line = true;
    return;
  }

  for (int line_position = 0; line_position < file_line.size();
       line_position++) {
    // search for ' ' split delimeter and parse property
    if (file_line[line_position] != ' ') {
      continue;
    }
    std::string type = file_line.substr(0, line_position);
    std::string value = file_line.substr(
        line_position + 1, file_line.size() - (line_position + 1));
    if (type == "type") {
      output_map->type = value;
    } else if (type == "height") {
      output_map->height = std::stoi(value);
    } else if (type == "width") {
      output_map->width = std::stoi(value);
    }
  }
}

MapPtr load_map(const std::filesystem::path &path) {
  std::fstream map_file(path, std::fstream::in);

  MapPtr output_map = std::make_shared<Map>();

  if (!map_file.good()) {
    printf("Can't open file %s\n", path.c_str());
    return nullptr;
  }

  std::string line;
  bool map_start = false;
  size_t row = 0;
  while (std::getline(map_file, line)) {
    if (!map_start) {
      parse_map_header_line(output_map, line, map_start);
    } else {
      for (size_t line_position = 0; line_position < line.size();
           line_position++) {
        int value = 0;
        switch (line[line_position]) {
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

        output_map->vertices[output_map->width * row + line_position] = value;
      }
      row++;
    }
  }
  return output_map;
}

CoordinateT get_distance(CoordinateT a, CoordinateT b) {
  return std::abs(a - b);
}

GraphPtr map_to_graph(const MapPtr map) {

  GraphPtr graph = std::make_shared<Graph>();
  graph->nodes = std::vector<GraphNodePtr>(map->height * map->width);
  graph->width = map->width;
  graph->height = map->height;

  for (size_t y = 0; y < map->height; y++) {
    for (size_t x = 0; x < map->width; x++) {
      const int node_index = y * map->width + x;
      GraphNodePtr node = std::make_shared<GraphNode>();
      node->value = map->get_vertex(x, y);
      node->x = x;
      node->y = y;
      graph->nodes[y * map->width + x] = node;

      if (node->value != 0) {
        continue;
      }

      auto top_node = graph->get_node(x, y - 1);
      if (top_node && top_node->value == 0) {
        node->edges.push_back(top_node);
        top_node->edges.push_back(node);
      }
      auto left_node = graph->get_node(x - 1, y);
      if (left_node && left_node->value == 0) {
        node->edges.push_back(left_node);
        left_node->edges.push_back(node);
      }
    }
  }

  return graph;
}
} // namespace mapf