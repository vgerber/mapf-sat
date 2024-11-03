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
  GraphNode *node = nullptr;
  float f = std::numeric_limits<float>()
                .max(); // Cost based on cost function (e.g. distance to target)
  float g = 0.0f;       // Cost based of num nodes visited
  SearchNode *parent = nullptr;
  std::vector<SearchNode *> successors;
};

/**
 * @brief Decoded MAPF CNF Edge/Vertex
 *
 */
struct MAPFNode {
  int agent = -1;
  int x1, y1;
  int x2, y2;
  int t1, t2;
  int vertex = -1;
  int edge = -1;

  void print() {
    if (agent != -1) {
      if (edge != -1) {
        printf("e(%i,%i,%i,(%i,%i)->(%i,%i))", agent, t1, t2, x1, y1, x2, y2);
      } else {
        printf("v(%i,%i,%i,%i)", agent, t1, x1, y1);
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
  void setGraph(Graph graph);

  /**
   * @brief Solve path problem for given agents
   *
   * @param agents
   * @return true
   * @return false
   */
  bool solve(std::vector<Agent> &agents);

  bool isSolved();

  int getSolvedT();

  /**
   * @brief Get the solution if problem is sat
   *
   * @return std::vector<MAPFAgent>
   */
  std::vector<MAPFAgent> getSolution();

  /**
   * @brief Reads solution from solution.sat file (debug)
   *
   */
  void readSolution();

  bool isMDDOptimized();

  void useMDDOptimization(bool optimize);

private:
  bool tegUseMDD = false;
  std::unique_ptr<Glucose::Solver> satSolver; // SAT Solver
  int maxT = 0;                  // maxT time steps for path problem
  Graph graph;                   // Graph for problem search
  std::vector<MAPFAgent> agents; // Agents on Graph
  bool solved = false;

  /**
   * @brief Returns true if path for all agents is found
   *
   * @return true
   * @return false
   */
  bool isSAT();

  /**
   * @brief Get the Path from start to end
   *
   * @param startX
   * @param startY
   * @param endX
   * @param endY
   * @param maxLength
   * @return std::vector<GraphNode*>
   */
  std::vector<GraphNode *> getPath(int startX, int startY, int endX, int endY,
                                   int maxLength = 50);

  /**
   * @brief Get the Time Expansion Graph for te given path
   *
   * @param path
   * @param maxT
   * @return TimeExpansionGraph
   */
  TimeExpansionGraph getTimeExpansionGraph(std::vector<GraphNode *> path,
                                           int maxT);

  /**
   * @brief Get the CNF encoding for maxT
   *
   * @param clauses
   * @param variables
   * @return std::string
   */
  std::string getCNFEncoding(int &clauses, int &variables);

  /**
   * @brief Get the Vertex CNF Encoding
   *
   * @param agentIndex
   * @param time
   * @param pathIndex
   * @param positive
   * @return std::string
   */
  std::string getVertexEncoding(int agentIndex, int time, int pathIndex,
                                bool positive = true);

  /**
   * @brief Get the Edge CNF Encoding
   *
   * @param agentIndex
   * @param time
   * @param nextTime
   * @param pathIndex
   * @param edgeIndex
   * @param positive
   * @return std::string
   */
  std::string getEdgeEncoding(int agentIndex, int time, int nextTime,
                              int pathIndex, int edgeIndex,
                              bool positive = true);

  /**
   * @brief Get the Node from the CNF Encoding
   *
   * @param value
   * @return MAPFNode
   */
  MAPFNode getNode(int value);

  /**
   * @brief Tests if agent2 has the same vertex like agent
   *
   * @param agent
   * @param pathIndex
   * @param agent2
   * @return int
   */
  int getVertex(int agent1, int time, int pathIndex, int agent2);
};
} // namespace mapf