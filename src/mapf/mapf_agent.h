#pragma once
#include "map_util.h"
#include "time_exapnsion_graph.h"

namespace mapf {

/**
 * @brief Agent start and end properties
 *
 */
struct Agent {
  int x, y, targetX, targetY;
};

/**
 * @brief Agent which contains a path from start to end
 *
 * Additional:
 *  - TEG
 *
 */
struct MAPFAgent : public Agent {
  TimeExpansionGraph teg;
  std::vector<GraphNode *> desiredPath;
  std::vector<GraphNode *> path;

  // cnf encoding offsets
  int variables = 0;

  /**
   * @brief Returns index if teg has the same node at timestep t
   *
   * @param node
   * @param t
   * @return int index if found or -1 if not
   */
  int hasVertex(GraphNode *node, int t);
};

} // namespace mapf