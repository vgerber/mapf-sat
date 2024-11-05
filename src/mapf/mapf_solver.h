#pragma once
#include "map_util.h"
#include "mapf_agent.h"
#include <limits>
#include <memory>

#include <glucose/core/Solver.h>

namespace mapf {
/**
 * @brief Node for path finding algorithm
 *
 */
struct SearchNode {
  GraphNodePtr node = nullptr;
  float f = std::numeric_limits<float>()
                .max(); // Cost based on cost function (e.g. distance to target)
  float g = 0.0f;       // Cost based of num nodes visited
  std::shared_ptr<SearchNode> parent = nullptr;
  std::vector<std::shared_ptr<SearchNode>> successors;
};
typedef std::shared_ptr<SearchNode> SearchNodePtr;

/**
 * @brief Decoded MAPF CNF Edge/Vertex
 *
 */
struct MAPFNode {
  int agent = -1;
  CoordinateT x1, y1;
  CoordinateT x2, y2;
  size_t t1, t2;
  int vertex = -1;
  int edge = -1;

  void print() {
    if (agent != -1) {
      if (edge != -1) {
        printf("e(%i,%lu,%lu,(%i,%i)->(%i,%i))", agent, t1, t2, x1, y1, x2, y2);
      } else {
        printf("v(%i,%lu,%i,%i)", agent, t1, x1, y1);
      }
    }
  }
};

class MAPFSolver {
public:
  int MaxSolutionTime = 100;

  /**
   * @brief Set Graph of current instance
   *
   * @param graph
   */
  void set_graph(const GraphPtr graph);

  void register_agent(const MAPFAgentPtr agent);

  bool solve();

  bool is_solved();

  int get_solved_time_constraint();

  /**
   * @brief Reads solution from solution.sat file (debug)
   *
   */
  void read_solution();

  bool is_mdd_optimized();

  void use_mdd_optimization(bool optimize);

private:
  bool teg_use_mdd = false;
  std::unique_ptr<Glucose::Solver> sat_solver; // SAT Solver
  size_t max_t = 0;                 // max_t time steps for path problem
  GraphPtr graph = nullptr;         // Graph for problem search
  std::vector<MAPFAgentPtr> agents; // Agents on Graph

  typedef Glucose::vec<Glucose::Lit> Clause;
  typedef std::vector<Clause> Clauses;

  /**
   * @brief Returns true if path for all agents is found
   *
   * @return true
   * @return false
   */
  bool is_sat();

  /**
   * @brief Get the Path from start to end
   *
   * @param start_x
   * @param start_y
   * @param end_x
   * @param end_y
   * @param max_length
   * @return std::vector<GraphNode*>
   */
  std::vector<GraphNodePtr> get_path(CoordinateT start_x, CoordinateT start_y,
                                     CoordinateT end_x, CoordinateT end_y,
                                     size_t max_length = 50);

  TimeExpansionGraph
  get_time_expansion_graph(const std::vector<GraphNodePtr> &path, size_t max_t);

  inline Glucose::Lit add_new_var(const Glucose::Lit &lit);
  void encode_to_sat_solver();

  Glucose::Lit get_cnf_vertex_encoding(size_t agent_index, size_t time,
                                       size_t path_index, bool positive = true);

  Glucose::Lit get_cnf_edge_encoding(size_t agent_index, size_t time,
                                     size_t next_time, size_t path_index,
                                     size_t edge_index, bool positive = true);

  MAPFNode decode_cnf_node(int cnf_encoded_node);

  /**
   * @brief Tests if agent2 has the same vertex like agent
   *
   * @param agent
   * @param path_index
   * @param agent2
   * @return int
   */
  int get_vertex(int agent1, int time, int path_index, int agent2);
};
} // namespace mapf