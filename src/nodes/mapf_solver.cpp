#include "mapf_solver.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

const char *SIGNAL_SOLVER_FINISHED = "solver_finished";

namespace godot {

void MAPFSolver::_bind_methods() {
  ClassDB::bind_method(D_METHOD("solve"), &MAPFSolver::solve);
  ClassDB::bind_method(D_METHOD("add_agent", "agent"), &MAPFSolver::add_agent);
  ClassDB::bind_method(D_METHOD("set_map", "map"), &MAPFSolver::set_map);
  ADD_SIGNAL(MethodInfo(SIGNAL_SOLVER_FINISHED,
                        PropertyInfo(Variant::BOOL, "solved")));
}

MAPFSolver::MAPFSolver() {}

MAPFSolver::~MAPFSolver() {}

void MAPFSolver::set_map(MAPFMap *map) {
  solver.set_graph(mapf::map_to_graph(map->get_map_handle()));
  agents.clear();
  solver.clear_agents();
}

void MAPFSolver::add_agent(MAPFAgent *agent) {
  agents.push_back(agent);
  solver.register_agent(agent->get_agent_handle());
}

void MAPFSolver::solve() {
  bool solved = solver.solve();

  emit_signal(SIGNAL_SOLVER_FINISHED, solved);

  if (!solved) {
    return;
  }

  for (const auto &agent : agents) {
    agent->reset_path_state();
  }
}

}; // namespace godot