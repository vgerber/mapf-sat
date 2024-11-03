#include "mapf_solver.h"
#include <algorithm>
#include <chrono>
#include <glucose/core/Dimacs.h>
#include <glucose/core/SolverTypes.h>
#include <iostream>

namespace mapf {

void MAPFSolver::setGraph(Graph graph) { this->graph = graph; }

bool MAPFSolver::solve(std::vector<Agent> &agents) {
  auto time_solve_start = std::chrono::system_clock::now();
  this->agents.clear();

  // true if problem is solved
  solved = false;

  // time adjust
  int deltaT = 0;

  // max of min desiredPath duration
  int minT = 0;

  // combined time (minT + dT)
  maxT = minT;

  // calculate initial values
  auto time_get_path_start = std::chrono::system_clock::now();
  for (auto agent : agents) {
    MAPFAgent mapfAgent;

    // get agent start and target
    mapfAgent.x = agent.x;
    mapfAgent.y = agent.y;
    mapfAgent.targetX = agent.targetX;
    mapfAgent.targetY = agent.targetY;

    // get agent desiredPath
    mapfAgent.desiredPath =
        getPath(agent.x, agent.y, agent.targetX, agent.targetY);

    // get max time from minimum paths
    minT = std::max(int(mapfAgent.desiredPath.size()), minT);

    // add agent to problem list
    this->agents.push_back(mapfAgent);
  }
  auto time_get_path_end = std::chrono::system_clock::now();

  // find solution until hard constraint is found
  while (maxT <= MaxSolutionTime) {
    maxT = minT + deltaT;

    // create TEGs
    for (auto &agent : this->agents) {
      agent.teg = getTimeExpansionGraph(agent.desiredPath, maxT);
    }

    if (isSAT()) {
      solved = true;
      break;
    } else {
      deltaT++;
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

bool MAPFSolver::isSolved() { return solved; }

int MAPFSolver::getSolvedT() { return maxT; }

bool MAPFSolver::isSAT() {

  // encode problem to cnf
  int clauses = 0;
  int variables = 0;
  std::string cnf = getCNFEncoding(clauses, variables);

  // write cnf to .cnf file
  std::string cnfFile = "";
  cnfFile += "c MAPF Encoding\n";
  cnfFile += "p cnf " + std::to_string(variables) + " " +
             std::to_string(clauses) + "\n";
  cnfFile += cnf;

  std::fstream cnfFileStream("mapf.cnf", std::fstream::out);
  if (cnfFileStream.good()) {
    cnfFileStream << cnfFile;
  }
  cnfFileStream.close();

  // read cnf file and solve with glucose sat solver
  satSolver = std::make_unique<Glucose::Solver>();

  gzFile in = gzopen("mapf.cnf", "rb");
  Glucose::parse_DIMACS(in, *satSolver);
  gzclose(in);

  // satSolver.eliminate(true);

  Glucose::vec<Glucose::Lit> dummy;
  Glucose::lbool isSAT = satSolver->solveLimited(dummy);

  // if problem is sat set solved to true
  if (isSAT != l_True) {
    printf("T=%u - MAPF is not satisfiable\n", maxT);
    return false;
  }
  std::vector<std::vector<GraphNode *>> newPaths;
  newPaths.resize(agents.size());
  for (int i = 0; i < satSolver->nVars(); i++) {
    if (satSolver->model[i] == l_True) {
      MAPFNode node = getNode(i + 1);

      if (node.edge == -1 && node.vertex >= 0) {
        newPaths[node.agent].push_back(graph.get(node.x1, node.y1));
      }
    }
  }
  for (int a = 0; a < agents.size(); a++) {
    agents[a].path = newPaths[a];
  }
  return true;
}

std::vector<MAPFAgent> MAPFSolver::getSolution() {
  if (solved) {
    return agents;
  }
  return std::vector<MAPFAgent>();
}

void MAPFSolver::readSolution() {
  std::fstream solFile("solution.sat", std::fstream::in);

  if (solFile.good()) {
    std::string line;
    int lineCount = 0;
    while (std::getline(solFile, line)) {
      if (lineCount == 0) {
        if (line != "SAT") {
          printf("MAPF is not satisfiable\n");
          break;
        }
      }
      if (lineCount == 1) {
        int varStart = 0;
        for (int i = 0; i < line.size(); i++) {
          if (line[i] == ' ' || i == line.size() - 1) {
            std::string variable = line.substr(varStart, i - varStart).c_str();
            varStart = i + 1;

            if (variable[0] != '-' && variable.size() > 0) {
              MAPFNode node = getNode(std::stoi(variable));
              if (node.vertex >= 0 && node.edge == -1) {
                node.print();
                printf("\n");
              }
            }
          }
        }
      }
      lineCount++;
    }
  } else {
    printf("Error while opening file\n");
  }
}

bool MAPFSolver::isMDDOptimized() { return tegUseMDD; }

void MAPFSolver::useMDDOptimization(bool optimize) { tegUseMDD = optimize; }

std::vector<GraphNode *> MAPFSolver::getPath(int startX, int startY, int endX,
                                             int endY, int maxLength) {
  std::vector<GraphNode *> desiredPath;
  GraphNode *startNode = nullptr;
  for (int i = 0; i < graph.nodes.size(); i++) {
    GraphNode *node = graph.nodes[i];
    if (node->x == startX && node->y == startY) {
      startNode = node;
    }
  }
  if (startNode) {
    std::vector<SearchNode *> unexplored;
    SearchNode *root = new SearchNode();
    root->node = startNode;
    root->f = 0.0f;
    unexplored.push_back(root);
    bool solutionFound = false;
    while (!solutionFound) {
      if (unexplored.size() == 0) {
        break;
      }
      SearchNode *currentNode = unexplored[0];

      if (currentNode->node->x == endX && currentNode->node->y == endY) {
        solutionFound = true;
        break;
      }

      unexplored.erase(unexplored.begin());

      // append new unvisited search nodes
      for (auto neighbor : currentNode->node->edges) {
        SearchNode *node = new SearchNode();
        node->node = neighbor;
        node->g = currentNode->g + 1.0f;
        node->f = node->g + abs(neighbor->x - endX) + abs(neighbor->y - endY);
        node->parent = currentNode;

        // stop at max  length
        if (node->g > maxLength) {
          delete node;
          continue;
        }

        // remove duplicates
        // check if there is any unexplored vertex with the same graph node
        // if this condition is true: remove the worst
        bool betterDuplicateFound = false;
        for (int i = 0; i < unexplored.size(); i++) {
          SearchNode *oldNode = unexplored[i];
          if (oldNode->node == node->node) {
            if (oldNode->f > node->f) {
              unexplored.erase(unexplored.begin() + i);
              i--;
            } else {
              betterDuplicateFound = true;
              break;
            }
          }
        }
        if (!betterDuplicateFound) {
          unexplored.push_back(node);
          currentNode->successors.push_back(node);
        } else {
          delete node;
        }
      }

      // sort unexplored by lowest cost first
      std::sort(unexplored.begin(), unexplored.end(),
                [](SearchNode *a, SearchNode *b) { return a->f < b->f; });
    }
    if (solutionFound) {
      SearchNode *pathNode = unexplored[0];
      desiredPath.push_back(pathNode->node);
      while (pathNode->parent) {
        pathNode = pathNode->parent;
        desiredPath.push_back(pathNode->node);
      }
      std::reverse(desiredPath.begin(), desiredPath.end());
    }
  }
  return desiredPath;
}

TimeExpansionGraph
MAPFSolver::getTimeExpansionGraph(std::vector<GraphNode *> desiredPath,
                                  int maxT) {
  TimeExpansionGraph teg;

  // add time expansions for each time step t
  for (int t = 0; t < maxT; t++) {
    int stepsLeft = maxT - (t + 1);
    std::vector<TimeExpansionNode> tegStep;
    int edges = 0;

    for (int i = 0; i < desiredPath.size(); i++) {
      TimeExpansionNode node;

      // add all adjacent nodes
      // if mdd is true then only add the edges of reachable nodes
      if (!(tegUseMDD && i > t) && t < maxT - 1) {

        for (int j = -1; j <= 1; j++) {
          int neighbourIndex = i + j;

          // if mdd is true only accept nodes which can reach the goal state in
          // maxT
          int requiredSteps = ((desiredPath.size() - 1) - neighbourIndex);
          if (tegUseMDD && stepsLeft <= requiredSteps) {
            continue;
          }

          if (neighbourIndex >= 0 && neighbourIndex < desiredPath.size()) {
            node.edgeNodes.push_back(neighbourIndex);
            edges++;
          }
        }
      }
      tegStep.push_back(node);
    }
    teg.teg.push_back(tegStep);
    teg.edges.push_back(edges);
  }
  return teg;
}

std::string MAPFSolver::getCNFEncoding(int &clauses, int &variables) {
  variables = 0;
  clauses = 0;
  std::string cnf = "";

  // calculate agent dimensions
  for (auto &agent : agents) {
    // calculate agent offset
    agent.variables = 0;
    for (int i = 0; i < maxT; i++) {
      agent.variables += agent.teg.teg[i].size();
      agent.variables += agent.teg.edges[i];
    }

    variables += agent.variables;
  }

  // encode for each agent
  for (int i = 0; i < agents.size(); i++) {
    MAPFAgent agent = agents[i];

    // RULE 0
    // start and end node
    cnf += getVertexEncoding(i, 0, 0) + " 0\n";
    cnf +=
        getVertexEncoding(i, maxT - 1, agent.desiredPath.size() - 1) + " 0\n";
    clauses += 2;
    for (int t = 0; t < maxT; t++) {

      for (int v = 0; v < agent.teg.teg[t].size(); v++) {
        // RULE 1
        // only one vertex at time step t
        for (int v2 = 0; v2 < agent.teg.teg[t].size(); v2++) {
          if (v2 == v) {
            continue;
          }
          cnf += getVertexEncoding(i, t, v, false) + " " +
                 getVertexEncoding(i, t, v2, false) + " 0\n";
          clauses++;
        }

        if (t < maxT - 1) {

          // leaving an vertex

          // RULE 1 a)
          // each agent has to leave his edge in the next time step
          cnf += getVertexEncoding(i, t, v, false);

          for (int e = 0; e < agent.teg.teg[t][v].edgeNodes.size(); e++) {
            cnf += " " + getEdgeEncoding(i, t, t + 1, v, e);
          }
          cnf += " 0\n";
          clauses++;

          // RULE 1 b)
          // each agent can only traverse one edge at the time
          for (int e1 = 0; e1 < agent.teg.teg[t][v].edgeNodes.size(); e1++) {
            for (int e2 = 0; e2 < agent.teg.teg[t][v].edgeNodes.size(); e2++) {
              if (e1 == e2) {
                continue;
              }
              cnf += getEdgeEncoding(i, t, t + 1, v, e1, false) + " " +
                     getEdgeEncoding(i, t, t + 1, v, e2, false) + " 0\n";

              clauses++;
            }
          }

          // RULE 2
          // edge e is blocked if agent i moves from x/y -> x'/y'
          for (int e = 0; e < agent.teg.teg[t][v].edgeNodes.size(); e++) {
            int v2 = agent.teg.teg[t][v].edgeNodes[e];
            std::string edgeEncoding =
                getEdgeEncoding(i, t, t + 1, v, e, false);
            cnf += edgeEncoding + " " + getVertexEncoding(i, t, v) + " 0\n";
            cnf +=
                edgeEncoding + " " + getVertexEncoding(i, t + 1, v2) + " 0\n";
            clauses += 2;
          }

          // RULE 4
          // no two agents at the same vertex
          for (int a = 0; a < agents.size(); a++) {
            if (a == i) {
              continue;
            }
            int agent2Vertex = agents[a].hasVertex(agent.desiredPath[v], t);

            if (agent2Vertex >= 0) {
              cnf += getVertexEncoding(a, t, agent2Vertex, false) + " " +
                     getVertexEncoding(i, t, v, false) + " 0\n";
              clauses += 1;
            }
          }
        }
      }
    }
  }
  return cnf;
}

std::string MAPFSolver::getVertexEncoding(int agentIndex, int time,
                                          int pathIndex, bool positive) {
  MAPFAgent agent = agents[agentIndex];
  // add offset for cnf encoding
  int value = 1;

  // add agent offset
  for (int a = 0; a < agentIndex; a++) {
    value += agents[a].variables;
  }

  // add time offset
  for (int t = 0; t < time; t++) {
    value += agent.teg.teg[t].size() + agent.teg.edges[t];
  }

  // add vertex offset
  value += pathIndex;

  return std::to_string(positive ? value : -value);
}

std::string MAPFSolver::getEdgeEncoding(int agentIndex, int time, int nextTime,
                                        int pathIndex, int edgeIndex,
                                        bool positive) {
  MAPFAgent agent = agents[agentIndex];
  // add offset for cnf encoding
  int value = 1;

  // add agent offset
  for (int a = 0; a < agentIndex; a++) {
    value += agents[a].variables;
  }

  // add time offset
  for (int t = 0; t < time; t++) {
    value += agent.teg.teg[t].size() + agent.teg.edges[t];
  }

  // add vertex offset
  value += agent.teg.teg[time].size();

  // add edge offset
  for (int v = 0; v < pathIndex; v++) {
    value += agent.teg.teg[time][v].edgeNodes.size();
  }
  value += edgeIndex;

  return std::to_string(positive ? value : -value);
}

MAPFNode MAPFSolver::getNode(int value) {
  value--; // remove offset by 1
  MAPFNode node;
  // find agent offset
  for (int i = 0; i < agents.size(); i++) {
    MAPFAgent agent = agents[i];
    if (value >= agent.variables) {
      value -= agent.variables;
    } else {
      node.agent = i;
      // find time slot
      for (int t = 0; t < maxT; t++) {
        int vertices = agent.teg.teg[t].size();
        int edges = agent.teg.edges[t];
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
              int edges = agent.teg.teg[t][v].edgeNodes.size();
              if (value - edges < 0) {
                break;
              }
              value -= edges;
            }

            node.vertex = v;
            node.edge = agent.teg.teg[t][v].edgeNodes[value];

            GraphNode *pathNode = agent.desiredPath[node.vertex];
            GraphNode *edgeNode = agent.desiredPath[node.edge];

            node.x1 = pathNode->x;
            node.y1 = pathNode->y;
            node.x2 = edgeNode->x;
            node.y2 = edgeNode->y;
            return node;
          } else { // vertex node
            node.agent = i;
            node.t1 = t;
            node.t2 = -1;
            node.vertex = value;
            GraphNode *pathNode = agent.desiredPath[node.vertex];
            node.x1 = pathNode->x;
            node.y1 = pathNode->y;
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

int MAPFSolver::getVertex(int agent1, int time, int pathIndex, int agent2) {
  TimeExpansionNode node = agents[agent1].teg.teg[time][pathIndex];
  return -1;
}
} // namespace mapf