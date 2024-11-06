#pragma once

#include "mapf/mapf_agent.h"
#include <godot_cpp/classes/node2d.hpp>
#include <memory>

namespace godot {

class MAPFAgent : public Node2D {
  GDCLASS(MAPFAgent, Node2D)

private:
  std::shared_ptr<mapf::MAPFAgent> agent;
  size_t path_index = 0;
  Vector2i cell_size;

protected:
  static void _bind_methods();

public:
  MAPFAgent();
  ~MAPFAgent();

  void set_cell(const Vector2i &cell);
  void set_target_cell(const Vector2i &cell);
  void set_cell_size(const Vector2i &cell_size);
  void reset_path_state();

  size_t get_path_length() const;
  Vector2i get_path_cell(size_t path_index) const;
  int get_current_path_index() const;
  void set_path(std::vector<mapf::GraphNodePtr> path);

  std::shared_ptr<mapf::MAPFAgent> get_agent_handle() const;

  void _process(float delta);
};

} // namespace godot