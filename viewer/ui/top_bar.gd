extends PanelContainer

signal solve_click
signal add_agent
signal map_selected(map_file_path: String)

var cameras: Array[Camera2D] = []
var agents: Array[Agent] = []

@onready var camera_select = $MarginContainer/HBoxContainer/CameraSelectBtn
@onready var agent_select = $MarginContainer/HBoxContainer/AgentSelectBtn
@onready var add_agent_btn = $MarginContainer/HBoxContainer/AddAgentBtn


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	add_agent_btn.pressed.connect(func(): add_agent.emit())

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func register_camera(camera: Camera2D, name: String) -> void:
	cameras.push_back(camera)
	camera_select.add_item(name)
	
	if camera.is_current():
		camera_select.select(cameras.size() - 1)

func register_agent(agent: Agent, name: String) -> void:
	agents.push_back(agent)
	agent_select.add_item(name)
	agent_select.select(agents.size() - 1)
	register_camera(agent.camera, name)
	
func unregister_agent(agent: Agent) -> void:
	var agent_index = agents.find(agent)
	agents.remove_at(agent_index)
	agent_select.remove_item(agent_index)

func _on_camera_select_btn_item_selected(index: int) -> void:
	cameras[index].make_current()


func _on_solve_btn_pressed() -> void:
	print("Solve!")
	solve_click.emit()


func _on_map_selector_map_selected(map_file: String) -> void:
	map_selected.emit(map_file)
