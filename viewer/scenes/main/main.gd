extends Node

@onready var map_view = $Control/SubViewportContainer/SubViewport/MapView
@onready var top_bar = $Control/TopBar

const agent_path_scene = preload("res://world/entities/agent/agent_path.tscn")


var solver = MAPFSolver.new()

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	#mapf_map.load_map("/home/vincent/Development/Godot/gdextension_test/demo/assets/pssai/mouse1.map")
	map_view.load_map(MapSelector.get_map_absolute_path("res://assets/maps/basic/mouse1.map"))
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
