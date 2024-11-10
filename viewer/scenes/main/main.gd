extends Node

@onready var map_view = $Control/HBoxContainer/SubViewportContainer/SubViewport/MapView
@onready var top_bar = $Control/TopBar
@onready var preset_list = $Control/HBoxContainer/PresetsList

const agent_path_scene = preload("res://world/entities/agent/agent_path.tscn")

var config = MAPFConfig.new([])
var solver = MAPFSolver.new()

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	#mapf_map.load_map("/home/vincent/Development/Godot/gdextension_test/demo/assets/pssai/mouse1.map")
	map_view.load_map(MapSelector.get_map_absolute_path("res://assets/maps/basic/mouse1.map")) as MapView
	solver.set_map(map_view.mapf_map)
	
	map_view.agent_added.connect(_on_agent_added)
	map_view.agent_removed.connect(top_bar.unregister_agent)
	
	top_bar.register_camera(map_view.camera, "Free")
	


func _on_top_bar_solve_click() -> void:
	solver.solve()

func _on_agent_added(agent: Agent) -> void:
	solver.add_agent(agent)
	top_bar.register_agent(agent, "Agent %s" % top_bar.agents.size())

func _on_top_bar_add_agent() -> void:
	map_view.switch_to_place_agent_action()


func _on_top_bar_map_selected(map_file_path: String) -> void:
	map_view.load_map(MapSelector.get_map_absolute_path(map_file_path))
	solver.set_map(map_view.mapf_map)


func _on_top_bar_show_presets_changed(show: bool) -> void:
	$Control/HBoxContainer/PresetsList.visible = show


func _on_presets_list_add_preset(preset_name: String) -> void:
	var config_agents: Array[MAPFConfig.ConfigAgent] = []
	config_agents.assign(map_view.agents.map(MAPFConfig.ConfigAgent.from_agent))
	var preset = MAPFConfig.ConfigPreset.new(preset_name, map_view.loaded_map_path, config_agents)

	config.presets.push_back(preset)
	preset_list.add_preset_to_list(preset)
	

func _on_presets_list_save_config(config_path: String) -> void:
	print("Save to " + config_path)
	FileAccess.open(config_path, FileAccess.WRITE).store_string(JSON.stringify(config.to_dict()))

func reload_presets():
	preset_list.clear_preset_list()
	for preset in config.presets:
		preset_list.add_preset_to_list(preset)

func _on_presets_list_load_config(config_path: String) -> void:
	print("Load from " + config_path)
	config = MAPFConfig.from_dict(JSON.parse_string(FileAccess.open(config_path, FileAccess.READ).get_as_text()))
	reload_presets()


func _on_presets_list_load_preset(preset: MAPFConfig.ConfigPreset) -> void:
	map_view.load_map(preset.map_path)
	for agent in preset.agents:
		map_view.add_agent_at_tile(Vector2i(agent.x, agent.y), Vector2i(agent.target_x, agent.target_y))


func _on_presets_list_remove_preset(preset: MAPFConfig.ConfigPreset) -> void:
	config.presets.remove_at(config.presets.find(preset))
	reload_presets()
