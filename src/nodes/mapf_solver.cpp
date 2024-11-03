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
  solver.setGraph(mapf::getGraph(*map->get_map_handle()));
}

void MAPFSolver::add_agent(MAPFAgent *agent) { agents.push_back(agent); }

void MAPFSolver::solve() {
  std::vector<mapf::Agent> agent_handles = {};
  for (const auto &agent : agents) {
    agent_handles.push_back(*agent->get_agent_handle());
  }
  bool solved = solver.solve(agent_handles);
  UtilityFunctions::print("Solver result ", solved);

  if (!solved) {
    return;
  }

  UtilityFunctions::print("Updating ", agent_handles.size(), " agents");
  if (solver.getSolution().size() != agents.size()) {
    UtilityFunctions::printerr("Solution does not match to agents ",
                               agent_handles.size(), " vs ",
                               solver.getSolution().size());
    return;
  }

  for (size_t agent_index = 0; agent_index < agent_handles.size();
       agent_index++) {
    UtilityFunctions::print("Updating agent  ", agent_index);
    const mapf::MAPFAgent updated_agent = solver.getSolution()[agent_index];
    MAPFAgent *agent = agents[agent_index];
    agent->set_path(updated_agent.path);
    agent->get_agent_handle()->teg = updated_agent.teg;

    for (auto path : updated_agent.path) {
      UtilityFunctions::print(path->x, ",", path->y);
    }

    agent->reset_path_state();
  }
}

}; // namespace godot