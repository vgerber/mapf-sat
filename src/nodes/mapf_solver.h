#pragma once

#include "mapf/mapf_solver.h"
#include "mapf_agent.h"
#include "mapf_map.h"
#include <godot_cpp/classes/object.hpp>

namespace godot {
using MAPFAgents = std::vector<MAPFAgent *>;

class MAPFSolver : public Object {
  GDCLASS(MAPFSolver, Object)

private:
  mapf::MAPFSolver solver;
  MAPFAgents agents;
  MAPFMap *map;

protected:
  static void _bind_methods();

public:
  MAPFSolver();
  ~MAPFSolver();

  void set_map(MAPFMap *map);
  void add_agent(MAPFAgent *agent);

  void solve();
};

} // namespace godot