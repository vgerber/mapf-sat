#include "mapf_solver.h"
#include <algorithm>
#include <chrono>
#include <glucose/core/Dimacs.h>
#include <glucose/core/SolverTypes.h>
#include <iostream>

namespace mapf {

void MAPFSolver::set_graph(const GraphPtr graph) { this->graph = graph; }

void MAPFSolver::register_agent(const MAPFAgentPtr agent) {
  agents.push_back(agent);
}

void MAPFSolver::clear_agents() { agents.clear(); }

bool MAPFSolver::solve() {
  std::cout << "Solving for " << agents.size() << " Agents" << std::endl;
  auto time_solve_start = std::chrono::system_clock::now();
  bool solved = false;

  // time adjust
  int delta_t = 0;

  // max of min desired_path duration
  int min_t = 0;

  // combined time (min_t + dT)
  max_t = min_t;

  // calculate initial values
  auto time_get_path_start = std::chrono::system_clock::now();
  for (auto agent : agents) {

    // get agent desired_path
    agent->desired_path =
        get_path(agent->x, agent->y, agent->target_x, agent->target_y);

    // get max time from minimum paths
    min_t = std::max(int(agent->desired_path.size()), min_t);

    if (agent->desired_path.size() == 0) {
      printf("Currently all agents need a valid path of at least 1 tile\n");
      return false;
    }
  }
  auto time_get_path_end = std::chrono::system_clock::now();
  std::cout << "MinT " << min_t << " steps" << std::endl;

  // find solution until hard constraint is found
  while (max_t <= MaxSolutionTime) {
    std::cout << "Solving with MaxT=" << max_t << " steps" << std::endl;
    max_t = min_t + delta_t;

    // create TEGs
    for (const auto &agent : this->agents) {
      std::cout << "Agent DesiredPathLength=" << agent->desired_path.size()
                << std::endl;
      agent->teg = get_time_expansion_graph(agent->desired_path, max_t);
    }

    if (is_sat()) {
      solved = true;
      break;
    } else {
      delta_t++;
    }
  }
  auto time_sat_solve_end = std::chrono::system_clock::now();
  auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                        time_sat_solve_end - time_solve_start)
                        .count();
  auto path_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                       time_get_path_end - time_get_path_start)
                       .count();
  auto sat_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                      time_sat_solve_end - time_get_path_end)
                      .count();
  std::cout << "MAPFSolver::solve total=" << total_time
            << "ms path=" << path_time << "ms sat=" << sat_time << "ms"
            << std::endl;

  return solved;
}

int MAPFSolver::get_solved_time_constraint() { return max_t; }

bool MAPFSolver::is_sat() {

  // encode problem to cnf
  sat_solver = std::make_unique<Glucose::Solver>();
  encode_to_sat_solver();

  printf("Generated CNF Agents=%lu Clauses=%i Variables=%i\n", agents.size(),
         sat_solver->nClauses(), sat_solver->nVars());

  Glucose::vec<Glucose::Lit> dummy;
  Glucose::lbool sat_solver_is_sat = sat_solver->solveLimited(dummy);

  // if problem is sat set solved to true
  if (sat_solver_is_sat != l_True) {
    printf("T=%lu - MAPF is not satisfiable\n", max_t);
    return false;
  }
  std::vector<std::vector<GraphNodePtr>> new_paths;
  new_paths.resize(agents.size());
  for (int i = 0; i < sat_solver->nVars(); i++) {
    if (sat_solver->model[i] == l_True) {
      MAPFNode node = decode_cnf_node(i + 1);

      if (node.edge == -1 && node.vertex >= 0) {
        new_paths[node.agent].push_back(graph->get_node(node.x1, node.y1));
      }
    }
  }
  for (int a = 0; a < agents.size(); a++) {
    agents[a]->path = new_paths[a];
  }
  return true;
}

void MAPFSolver::read_solution() {
  std::fstream solFile("solution.sat", std::fstream::in);

  if (solFile.good()) {
    std::string line;
    int line_count = 0;
    while (std::getline(solFile, line)) {
      if (line_count == 0) {
        if (line != "SAT") {
          printf("MAPF is not satisfiable\n");
          break;
        }
      }
      if (line_count == 1) {
        int var_start = 0;
        for (int i = 0; i < line.size(); i++) {
          if (line[i] == ' ' || i == line.size() - 1) {
            std::string variable =
                line.substr(var_start, i - var_start).c_str();
            var_start = i + 1;

            if (variable[0] != '-' && variable.size() > 0) {
              MAPFNode node = decode_cnf_node(std::stoi(variable));
              if (node.vertex >= 0 && node.edge == -1) {
                node.print();
                printf("\n");
              }
            }
          }
        }
      }
      line_count++;
    }
  } else {
    printf("Error while opening file\n");
  }
}

bool MAPFSolver::is_mdd_optimized() { return teg_use_mdd; }

void MAPFSolver::use_mdd_optimization(bool optimize) { teg_use_mdd = optimize; }

std::vector<GraphNodePtr>
MAPFSolver::get_path(CoordinateT start_x, CoordinateT start_y,
                     CoordinateT end_x, CoordinateT end_y, size_t max_length) {
  std::vector<GraphNodePtr> desired_path;
  GraphNodePtr start_node = graph->get_node(start_x, start_y);
  if (!start_node) {
    printf("No start node found (%i,%i)-(%i,%i)\n", start_x, start_y, end_x,
           end_y);
    return {};
  }
  std::vector<SearchNodePtr> unexplored;
  SearchNodePtr root = std::make_shared<SearchNode>();
  root->node = start_node;
  root->f = 0.0f;
  unexplored.push_back(root);
  bool solution_found = false;
  while (!solution_found) {
    if (unexplored.size() == 0) {
      break;
    }
    SearchNodePtr current_node = unexplored[0];

    if (current_node->node->x == end_x && current_node->node->y == end_y) {
      solution_found = true;
      break;
    }

    unexplored.erase(unexplored.begin());

    // append new unvisited search nodes
    for (auto neighbor : current_node->node->edges) {
      SearchNodePtr node = std::make_shared<SearchNode>();
      node->node = neighbor;
      node->g = current_node->g + 1.0f;
      node->f = node->g + get_distance(neighbor->x, end_x) +
                get_distance(neighbor->y, end_y);
      node->parent = current_node;

      // stop at max  length
      if (node->g > max_length) {
        continue;
      }

      // remove duplicates
      // check if there is any unexplored vertex with the same graph node
      // if this condition is true: remove the worst
      bool better_duplicate_found = false;
      for (int i = 0; i < unexplored.size(); i++) {
        SearchNodePtr old_node = unexplored[i];
        if (old_node->node == node->node) {
          if (old_node->f > node->f) {
            unexplored.erase(unexplored.begin() + i);
            i--;
          } else {
            better_duplicate_found = true;
            break;
          }
        }
      }
      if (!better_duplicate_found) {
        unexplored.push_back(node);
        current_node->successors.push_back(node);
      }
    }

    // sort unexplored by lowest cost first
    std::sort(unexplored.begin(), unexplored.end(),
              [](SearchNodePtr a, SearchNodePtr b) { return a->f < b->f; });
  }
  if (!solution_found) {
    printf("No path found (%i,%i)-(%i,%i)\n", start_x, start_y, end_x, end_y);
    return {};
  }
  SearchNodePtr path_node = unexplored[0];
  desired_path.push_back(path_node->node);
  while (path_node->parent) {
    path_node = path_node->parent;
    desired_path.push_back(path_node->node);
  }
  std::reverse(desired_path.begin(), desired_path.end());

  return desired_path;
}

TimeExpansionGraph MAPFSolver::get_time_expansion_graph(
    const std::vector<GraphNodePtr> &desired_path, size_t max_t) {
  TimeExpansionGraph teg;

  // add time expansions for each time step t
  for (int t = 0; t < max_t; t++) {
    int steps_left = max_t - (t + 1);
    std::vector<TimeExpansionNode> teg_step;
    int edges = 0;

    for (int i = 0; i < desired_path.size(); i++) {
      TimeExpansionNode node;

      // add all adjacent nodes
      // if mdd is true then only add the edges of reachable nodes
      if (!(teg_use_mdd && i > t) && t < max_t - 1) {

        for (int j = -1; j <= 1; j++) {
          int neighbour_index = i + j;

          // if mdd is true only accept nodes which can reach the goal state in
          // max_t
          int required_steps = ((desired_path.size() - 1) - neighbour_index);
          if (teg_use_mdd && steps_left <= required_steps) {
            continue;
          }

          if (neighbour_index >= 0 && neighbour_index < desired_path.size()) {
            node.edge_nodes.push_back(neighbour_index);
            edges++;
          }
        }
      }
      teg_step.push_back(node);
    }
    teg.teg.push_back(teg_step);
    teg.edges.push_back(edges);
  }
  return teg;
}

Glucose::Lit MAPFSolver::add_new_var(const Glucose::Lit &lit) {
  while ((sat_solver->nVars() - 1) < abs(lit.x)) {
    sat_solver->newVar();
  }
  return lit;
}

void MAPFSolver::encode_to_sat_solver() {
  // calculate agent dimensions
  for (auto &agent : agents) {
    // calculate agent offset
    agent->variables = 0;
    for (int i = 0; i < max_t; i++) {
      agent->variables += agent->teg.teg[i].size();
      agent->variables += agent->teg.edges[i];
    }
  }

  // encode for each agent
  for (int agent_index = 0; agent_index < agents.size(); agent_index++) {
    MAPFAgentPtr agent = agents[agent_index];

    // RULE 0
    // start and end node
    sat_solver->addClause(
        add_new_var(get_cnf_vertex_encoding(agent_index, 0, 0)));

    sat_solver->addClause(add_new_var(get_cnf_vertex_encoding(
        agent_index, max_t - 1, agent->desired_path.size() - 1)));

    for (int t = 0; t < max_t; t++) {

      for (int v = 0; v < agent->teg.teg[t].size(); v++) {
        // RULE 1
        // only one vertex at time step t
        for (int v2 = 0; v2 < agent->teg.teg[t].size(); v2++) {
          if (v2 == v) {
            continue;
          }
          sat_solver->addClause(
              add_new_var(get_cnf_vertex_encoding(agent_index, t, v, false)),
              add_new_var(get_cnf_vertex_encoding(agent_index, t, v2, false)));
        }

        if (t < max_t - 1) {

          // leaving an vertex

          // RULE 1 a)
          // each agent has to leave his edge in the next time step
          Clause clause(agent->teg.teg[t][v].edge_nodes.size() + 1);
          clause[0] =
              add_new_var(get_cnf_vertex_encoding(agent_index, t, v, false));
          for (int e = 0; e < clause.size(); e++) {
            clause[e + 1] =
                add_new_var(get_cnf_edge_encoding(agent_index, t, t + 1, v, e));
          }
          sat_solver->addClause(clause);

          // RULE 1 b)
          // each agent can only traverse one edge at the time
          for (int e1 = 0; e1 < agent->teg.teg[t][v].edge_nodes.size(); e1++) {
            for (int e2 = 0; e2 < agent->teg.teg[t][v].edge_nodes.size();
                 e2++) {
              if (e1 == e2) {
                continue;
              }
              sat_solver->addClause(add_new_var(get_cnf_edge_encoding(
                                        agent_index, t, t + 1, v, e1, false)),
                                    add_new_var(get_cnf_edge_encoding(
                                        agent_index, t, t + 1, v, e2, false)));
            }
          }

          // RULE 2
          // edge e is blocked if agent i moves from x/y -> x'/y'
          for (int e = 0; e < agent->teg.teg[t][v].edge_nodes.size(); e++) {
            int v2 = agent->teg.teg[t][v].edge_nodes[e];
            Glucose::Lit edge_encoding = add_new_var(
                get_cnf_edge_encoding(agent_index, t, t + 1, v, e, false));
            sat_solver->addClause(
                edge_encoding,
                add_new_var(get_cnf_vertex_encoding(agent_index, t, v)));
            sat_solver->addClause(
                edge_encoding,
                add_new_var(get_cnf_vertex_encoding(agent_index, t + 1, v2)));
          }

          // RULE 4
          // no two agents at the same vertex
          for (int a = 0; a < agents.size(); a++) {
            if (a == agent_index) {
              continue;
            }
            std::optional<size_t> agent_vertex_index =
                agents[a]->get_vertex_index_at_time_step(agent->desired_path[v],
                                                         t);
            if (!agent_vertex_index) {
              continue;
            }

            sat_solver->addClause(
                add_new_var(
                    get_cnf_vertex_encoding(a, t, *agent_vertex_index, false)),
                add_new_var(get_cnf_vertex_encoding(agent_index, t, v, false)));
          }
        }
      }
    }
  }
}

Glucose::Lit MAPFSolver::get_cnf_vertex_encoding(size_t agent_index,
                                                 size_t time, size_t path_index,
                                                 bool positive) {
  MAPFAgentPtr agent = agents[agent_index];
  // add offset for cnf encoding
  int value = 1;

  // add agent offset
  for (int a = 0; a < agent_index; a++) {
    value += agents[a]->variables;
  }

  // add time offset
  for (int t = 0; t < time; t++) {
    value += agent->teg.teg[t].size() + agent->teg.edges[t];
  }

  // add vertex offset
  value += path_index;

  return positive ? Glucose::mkLit(value) : ~Glucose::mkLit(value);
}

Glucose::Lit MAPFSolver::get_cnf_edge_encoding(size_t agent_index, size_t time,
                                               size_t nextTime,
                                               size_t path_index,
                                               size_t edge_index,
                                               bool positive) {
  MAPFAgentPtr agent = agents[agent_index];
  // add offset for cnf encoding
  int value = 1;

  // add agent offset
  for (int a = 0; a < agent_index; a++) {
    value += agents[a]->variables;
  }

  // add time offset
  for (int t = 0; t < time; t++) {
    value += agent->teg.teg[t].size() + agent->teg.edges[t];
  }

  // add vertex offset
  value += agent->teg.teg[time].size();

  // add edge offset
  for (int v = 0; v < path_index; v++) {
    value += agent->teg.teg[time][v].edge_nodes.size();
  }
  value += edge_index;

  return positive ? Glucose::mkLit(value) : ~Glucose::mkLit(value);
  ;
}

MAPFNode MAPFSolver::decode_cnf_node(int value) {
  value--; // remove offset by 1
  MAPFNode node;
  // find agent offset
  for (int i = 0; i < agents.size(); i++) {
    MAPFAgentPtr agent = agents[i];
    if (value >= agent->variables) {
      value -= agent->variables;
    } else {
      node.agent = i;
      // find time slot
      for (int t = 0; t < max_t; t++) {
        int vertices = agent->teg.teg[t].size();
        int edges = agent->teg.edges[t];
        if (value >= (vertices + edges)) {
          value -= (vertices + edges);
        } else {
          node.t1 = t;
          node.t2 = t + 1;
          // check if value is in vertex region or edge region
          if (value >= vertices) { // edge node
            value -= vertices;
            int v = 0;
            for (v = 0; v < vertices; v++) {
              int edges = agent->teg.teg[t][v].edge_nodes.size();
              if (value - edges < 0) {
                break;
              }
              value -= edges;
            }

            node.vertex = v;
            node.edge = agent->teg.teg[t][v].edge_nodes[value];

            GraphNodePtr path_node = agent->desired_path[node.vertex];
            GraphNodePtr edgeNode = agent->desired_path[node.edge];

            node.x1 = path_node->x;
            node.y1 = path_node->y;
            node.x2 = edgeNode->x;
            node.y2 = edgeNode->y;
            return node;
          } else { // vertex node
            node.agent = i;
            node.t1 = t;
            node.t2 = -1;
            node.vertex = value;
            GraphNodePtr path_node = agent->desired_path[node.vertex];
            node.x1 = path_node->x;
            node.y1 = path_node->y;
            node.x2 = -1;
            node.y2 = -1;
            node.edge = -1;
            return node;
          }
        }
      }
    }
  }
  node.agent = -1;
  return node;
}

int MAPFSolver::get_vertex(int agent1, int time, int path_index, int agent2) {
  TimeExpansionNode node = agents[agent1]->teg.teg[time][path_index];
  return -1;
}
} // namespace mapf