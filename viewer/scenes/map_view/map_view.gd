class_name MapView
extends Node

signal agent_added(agent: Agent)

const agent_scene = preload("res://world/entities/agent/agent.tscn")
const agent_path_scene = preload("res://world/entities/agent/agent_path.tscn")
const agent_path_target_scene = preload("res://world/entities/agent/target_location.tscn")

enum MapAction { SELECT_AGENT = 0, NAVIGATE_AGENT = 1, PLACE_AGENT = 2 }

@onready var mapf_map = $MAPFMap
@onready var camera = $FreeCamera
@onready var cursor_rect = $CursorPosition

@export var cursor_select_texture: Texture2D
@export var cursor_navigate_texture: Texture2D
@export var cursor_place_texture: Texture2D

var agent_selector = preload("res://world/entities/agent/agent_selector.tscn").instantiate() as AgentSelector


var selected_agent: MAPFAgent = null
var current_action: MapAction = MapAction.SELECT_AGENT

var agents: Array[Agent] = []

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	get_viewport().size_changed.connect(func(): fit_camera_to_scene())
	_set_current_action(current_action)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func _unhandled_input(event: InputEvent) -> void:
	if not mapf_map:
		return
	if event is InputEventMouseMotion:
		unhandled_mouse_motion(event)
	if event is InputEventMouseButton:
		unhandled_mouse_button(event)

func unhandled_mouse_motion(event: InputEventMouseMotion) -> void:
	var tile_size: Vector2i = mapf_map.get_tile_set().tile_size
	cursor_rect.position = get_global_mouse_tile_position() * tile_size
	
func unhandled_mouse_button(event: InputEventMouseButton) -> void:
	if event.pressed:
		return
	
	var tile_size: Vector2i = mapf_map.get_tile_set().tile_size
	cursor_rect.position = get_global_mouse_tile_position() * tile_size
	
	if current_action == MapAction.PLACE_AGENT:
		add_agent_at_tile(get_global_mouse_tile_position())
	if current_action == MapAction.NAVIGATE_AGENT:
		selected_agent.set_target_cell(get_global_mouse_tile_position())
		unselect_agent()

func _on_agent_pressed(agent: Agent) -> void:
	if current_action != MapAction.SELECT_AGENT:
		return
	select_agent(agent)

func unselect_agent() -> void:
	_set_current_action(MapAction.SELECT_AGENT)
	selected_agent = null
	agent_selector.assign_agent(null)

func select_agent(agent: Agent) -> void:
	_set_current_action(MapAction.NAVIGATE_AGENT)
	selected_agent = agent
	agent_selector.assign_agent(selected_agent)

func _on_mapf_map_map_changed() -> void:
	fit_camera_to_scene()
	
func get_global_mouse_tile_position() -> Vector2i:
	var pos = mapf_map.get_global_mouse_position()
	var tile_size: Vector2i = mapf_map.get_tile_set().tile_size
	return Vector2i(pos.x, pos.y) / tile_size
		
func fit_camera_to_scene() -> void:
	var map_size = mapf_map.get_used_rect().size
	var tile_size = mapf_map.get_tile_set().tile_size
	var map_size_px = map_size * tile_size
	camera.position = map_size_px / 2 + mapf_map.get_used_rect().position * tile_size
	
	var window_size = get_viewport().size
	
	var zoom = min(float(window_size.x) / map_size_px.x, float(window_size.y) / map_size_px.y)
	camera.zoom = Vector2(zoom, zoom)

func add_agent_at_tile(cell: Vector2i) -> void:
	var agent = agent_scene.instantiate() as Agent
	agent.name = str(agents.size())
	
	agent.set_cell(cell)
	agent.set_cell_size(mapf_map.get_tile_set().tile_size)
	agent.set_target_cell(cell)
	agent.pressed.connect(func(): _on_agent_pressed(agent))
	agent.z_index = 10
	mapf_map.add_child(agent)
	agent_added.emit(agent)
	
	var agent_path = agent_path_scene.instantiate() as AgentPath
	agent_path.assigned_agent = agent
	agent_path.z_index = 0
	mapf_map.add_child(agent_path)
	
	var agent_target = agent_path_target_scene.instantiate() as TargetLocation
	agent_target.modulate = agent.body.modulate
	agent_target.z_index = 9
	agent_target.hide()
	mapf_map.add_child(agent_target)
	agent_target.assign_agent(agent)
	
	select_agent(agent)
	agents.push_back(agent)
	_set_current_action(MapAction.NAVIGATE_AGENT)

func load_map(path: String) -> void:
	mapf_map.load_map(path)
	fit_camera_to_scene()

func switch_to_place_agent_action() -> void:
	selected_agent = null
	_set_current_action(MapAction.PLACE_AGENT)

func _set_current_action(map_action: MapAction) -> void:
	current_action = map_action
	if current_action == MapAction.SELECT_AGENT:
		cursor_rect.texture = cursor_select_texture
	if current_action == MapAction.NAVIGATE_AGENT:
		cursor_rect.texture = cursor_navigate_texture
	if current_action == MapAction.PLACE_AGENT:
		cursor_rect.texture = cursor_place_texture
