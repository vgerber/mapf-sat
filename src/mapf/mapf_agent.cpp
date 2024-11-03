#include "mapf_agent.h"

namespace mapf {

std::optional<size_t>
MAPFAgent::get_vertex_index_at_time_step(const GraphNodePtr node,
                                         size_t t) const {
  size_t index = 0;
  for (auto path_node : desired_path) {
    if (node->x == path_node->x && node->y == path_node->y) {
      return index;
    }
    index++;
  }
  return std::nullopt;
}

} // namespace mapf