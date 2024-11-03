#include "nodes/mapf_agent.h"
#include "nodes/mapf_map.h"
#include "nodes/mapf_solver.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_mapf_sat_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }

  GDREGISTER_CLASS(MAPFMap);
  GDREGISTER_CLASS(MAPFAgent);
  GDREGISTER_CLASS(MAPFSolver);
}

void uninitialize_mapf_sat_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
GDExtensionBool GDE_EXPORT
mapf_sat_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                      const GDExtensionClassLibraryPtr p_library,
                      GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_mapf_sat_module);
  init_obj.register_terminator(uninitialize_mapf_sat_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}