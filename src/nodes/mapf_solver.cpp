#include "mapf_solver.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void MAPFSolver::_bind_methods() {
  ClassDB::bind_method(D_METHOD("solve"), &MAPFSolver::solve);
  ClassDB::bind_method(D_METHOD("add_agent", "agent"), &MAPFSolver::add_agent);
  ClassDB::bind_method(D_METHOD("set_map", "map"), &MAPFSolver::set_map);
}

MAPFSolver::MAPFSolver() {}

MAPFSolver::~MAPFSolver() {}

void MAPFSolver::set_map(MAPFMap *map) {
  solver.set_graph(mapf::map_to_graph(map->get_map_handle()));
}

void MAPFSolver::add_agent(MAPFAgent *agent) {
  agents.push_back(agent);
  solver.register_agent(agent->get_agent_handle());
}

void MAPFSolver::solve() {
  bool solved = solver.solve();
  UtilityFunctions::print("Solver result ", solved);

  if (!solved) {
    return;
  }

  for (size_t agent_index = 0; agent_index < agents.size(); agent_index++) {
    UtilityFunctions::print("Updating agent  ", agent_index);
    auto agent = agents[agent_index];

    for (auto path : agent->get_agent_handle()->path) {
      UtilityFunctions::print(path->x, ",", path->y);
    }

    agent->reset_path_state();
  }
}

}; // namespace godot