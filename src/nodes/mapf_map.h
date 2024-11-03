#pragma once
#include "mapf/map_util.h"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <memory>

namespace godot {
class MAPFMap : public Node2D {
  GDCLASS(MAPFMap, Node2D)

private:
  TileMapLayer *tile_map_layer = nullptr;
  std::shared_ptr<mapf::Map> loaded_map = nullptr;

  void load_map_into_tile_map();

protected:
  static void _bind_methods();

public:
  MAPFMap();
  ~MAPFMap();

  void load_map(String map_path);
  int get_cell_source_id(const Vector2i cell) const;
  void set_tile_set(const Ref<TileSet> tile_set);
  Ref<TileSet> get_tile_set() const;
  Rect2i get_used_rect() const;

  std::shared_ptr<mapf::Map> get_map_handle();

  void _ready();
};
} // namespace godot
