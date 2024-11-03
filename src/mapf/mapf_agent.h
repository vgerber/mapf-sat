#pragma once
#include "map_util.h"
#include "time_exapnsion_graph.h"
#include <optional>

namespace mapf {

/**
 * @brief Agent start and end properties
 *
 */
struct Agent {
  size_t x, y, target_x, target_y;
};
typedef std::shared_ptr<Agent> AgentPtr;

/**
 * @brief Agent which contains a path from start to end
 *
 * Additional:
 *  - TEG
 *
 */
struct MAPFAgent : public Agent {
  TimeExpansionGraph teg;
  std::vector<GraphNodePtr> desired_path;
  std::vector<GraphNodePtr> path;

  // cnf encoding offsets
  int variables = 0;

  std::optional<size_t> get_vertex_index_at_time_step(const GraphNodePtr node,
                                                      size_t t) const;
};
typedef std::shared_ptr<MAPFAgent> MAPFAgentPtr;

} // namespace mapf