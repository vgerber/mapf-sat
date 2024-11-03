#include "mapf_map.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

const char *SIGNAL_MAP_CHANGED = "map_changed";

namespace godot {

void MAPFMap::_bind_methods() {

  ClassDB::bind_method(D_METHOD("set_tile_set", "tile_set"),
                       &MAPFMap::set_tile_set);
  ClassDB::bind_method(D_METHOD("get_tile_set"), &MAPFMap::get_tile_set);
  ClassDB::bind_method(D_METHOD("get_used_rect"), &MAPFMap::get_used_rect);
  ClassDB::bind_method(D_METHOD("get_cell_source_id"),
                       &MAPFMap::get_cell_source_id);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set",
                            PROPERTY_HINT_RESOURCE_TYPE, "TileSet"),
               "set_tile_set", "get_tile_set");

  ClassDB::bind_method(D_METHOD("load_map", "path"), &MAPFMap::load_map);
  ADD_SIGNAL(MethodInfo(SIGNAL_MAP_CHANGED));
}

MAPFMap::MAPFMap() { tile_map_layer = memnew(TileMapLayer); }

MAPFMap::~MAPFMap() {}

void MAPFMap::_ready() { add_child(tile_map_layer); }

void MAPFMap::load_map_into_tile_map() {
  UtilityFunctions::print("Loading tile map ", loaded_map->width, " ",
                          loaded_map->height);
  tile_map_layer->clear();

  TypedArray<Vector2i> added_cells;
  for (size_t x = 0; x < loaded_map->width; x++) {
    for (size_t y = 0; y < loaded_map->height; y++) {
      const int vertex_type = loaded_map->get_vertex(x, y) == 0 ? 0 : -1;
      if (vertex_type != 0) {
        continue;
      }
      added_cells.append(Vector2i(x, y));
    }
  }
  tile_map_layer->set_cells_terrain_connect(added_cells, 0, 0);
}

void MAPFMap::load_map(String map_path) {

  this->loaded_map = mapf::load_map(map_path.ascii().get_data());

  if (!this->loaded_map) {
    UtilityFunctions::print("Map load failed, clearing tile map");
    tile_map_layer->clear();
    return;
  }

  load_map_into_tile_map();

  emit_signal(SIGNAL_MAP_CHANGED);
}

int MAPFMap::get_cell_source_id(const Vector2i cell) const {
  return loaded_map->get_vertex(cell.x, cell.y);
}

void MAPFMap::set_tile_set(const Ref<TileSet> tile_set) {
  if (!tile_map_layer) {
    return;
  }
  tile_map_layer->set_tile_set(tile_set);
}

Ref<TileSet> MAPFMap::get_tile_set() const {
  if (!tile_map_layer) {
    return {};
  }
  return tile_map_layer->get_tile_set();
}

Rect2i MAPFMap::get_used_rect() const {
  return tile_map_layer->get_used_rect();
}

std::shared_ptr<mapf::Map> MAPFMap::get_map_handle() { return loaded_map; }

} // namespace godot
