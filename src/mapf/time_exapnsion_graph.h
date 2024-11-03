#pragma once
#include <vector>

/**
 * @brief Node for TEG
 *
 */
struct TimeExpansionNode {
  std::vector<unsigned int> edge_nodes;
};

/**
 * @brief Container for TEG
 *
 */
struct TimeExpansionGraph {
  std::vector<int> edges;
  std::vector<std::vector<TimeExpansionNode>> teg;
};