#include "mapf_agent.h"

namespace mapf {

int MAPFAgent::hasVertex(GraphNode *node, int t) {
  int index = 0;
  for (auto pathNode : desiredPath) {
    if (node->x == pathNode->x && node->y == pathNode->y) {
      return index;
    }
    index++;
  }
  return -1;
}

} // namespace mapf