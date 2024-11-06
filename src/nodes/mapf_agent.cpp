#include "mapf_agent.h"
#include <algorithm>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

const char *SIGNAL_PATH_CHANGED = "path_changed";
const char *SIGNAL_PATH_INDEX_CHANGED = "path_index_changed";

namespace godot {

void MAPFAgent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_cell", "cell"), &MAPFAgent::set_cell);
  ClassDB::bind_method(D_METHOD("set_cell_size", "size"),
                       &MAPFAgent::set_cell_size);
  ClassDB::bind_method(D_METHOD("set_target_cell", "cell"),
                       &MAPFAgent::set_target_cell);

  ClassDB::bind_method(D_METHOD("get_path_length"),
                       &MAPFAgent::get_path_length);
  ClassDB::bind_method(D_METHOD("get_path_cell", "path_index"),
                       &MAPFAgent::get_path_cell);
  ClassDB::bind_method(D_METHOD("get_current_path_index"),
                       &MAPFAgent::get_current_path_index);
  ADD_SIGNAL(MethodInfo(SIGNAL_PATH_CHANGED));
  ADD_SIGNAL(MethodInfo(SIGNAL_PATH_INDEX_CHANGED,
                        PropertyInfo(Variant::INT, "path_index")));
}

MAPFAgent::MAPFAgent() { agent = std::make_shared<mapf::MAPFAgent>(); }
MAPFAgent::~MAPFAgent() {}

void MAPFAgent::set_cell(const Vector2i &cell) {
  agent->x = cell.x;
  agent->y = cell.y;
  set_position(Vector2i(agent->x, agent->y) * cell_size);
}

void MAPFAgent::set_target_cell(const Vector2i &cell) {
  agent->target_x = cell.x;
  agent->target_y = cell.y;
}

void MAPFAgent::set_cell_size(const Vector2i &cell_size) {
  this->cell_size = cell_size;
  set_position(Vector2i(agent->x, agent->y) * cell_size);
}

void MAPFAgent::reset_path_state() {
  path_index = 0;
  emit_signal(SIGNAL_PATH_CHANGED);
}

size_t MAPFAgent::get_path_length() const { return agent->path.size(); }

Vector2i MAPFAgent::get_path_cell(size_t path_index) const {
  if (path_index > agent->path.size()) {
    throw std::runtime_error("Path index is out of bounds");
  }

  auto path_node = agent->path[path_index];
  return Vector2i(path_node->x, path_node->y);
}

int MAPFAgent::get_current_path_index() const { return path_index; }

void MAPFAgent::set_path(std::vector<mapf::GraphNodePtr> path) {
  agent->path = path;
  emit_signal(SIGNAL_PATH_CHANGED);
}

std::shared_ptr<mapf::MAPFAgent> MAPFAgent::get_agent_handle() const {
  return agent;
}

void MAPFAgent::_process(float delta) {
  if (path_index >= agent->path.size()) {
    return;
  }

  const float speed = 200;
  const mapf::GraphNodePtr next_node = agent->path[path_index];
  const Vector2i next_position =
      Vector2i(next_node->x, next_node->y) * cell_size;

  Vector2 direction =
      (Vector2(next_position.x, next_position.y) - get_position());

  set_position(get_position() + direction.normalized() * delta * speed);

  if (next_position.distance_to(get_position()) < 10) {
    agent->x = next_node->x;
    agent->y = next_node->y;
    set_position(next_position);
    path_index++;

    if (path_index < agent->path.size()) {
      const mapf::GraphNodePtr next_node = agent->path[path_index];
      const Vector2i next_position =
          Vector2i(next_node->x, next_node->y) * cell_size;
      // UtilityFunctions::print("Go to (", next_position.x, ",",
      // next_position.y,
      //                         ") ",
      //                         next_position.distance_to(get_position()));
      emit_signal(SIGNAL_PATH_INDEX_CHANGED, path_index);
    }
  }
}

} // namespace godot