extends PanelContainer

signal solve_click
signal add_agent
signal map_selected(map_file_path: String)
signal show_presets_changed(show: bool)

signal load_config(config_path: String)
signal save_config(config_path: String)

var cameras: Array[Camera2D] = []
var agents: Array[Agent] = []

@onready var camera_select = $MarginContainer/HBoxContainer/CameraSelectBtn
@onready var add_agent_btn = $MarginContainer/HBoxContainer/AddAgentBtn
@onready var config_menu_btn = $MarginContainer/HBoxContainer/ConfigMenu
@onready var show_presets_toggle_btn = $MarginContainer/HBoxContainer/ShowPresetsToggleBtn
@onready var map_selector = $MarginContainer/HBoxContainer/MapSelector

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	add_agent_btn.pressed.connect(func(): add_agent.emit())
	
	var config_menu: PopupMenu = config_menu_btn.get_popup()
	config_menu.add_item("Save...", 0)
	config_menu.add_item("Load...", 1)
	config_menu.id_pressed.connect(_on_config_menu_id_pressed)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func _on_config_menu_id_pressed(id: int) -> void:
	if id == 0:
		var file_dialog = FileDialog.new()
		file_dialog.add_filter("*.json", "JSON")
		file_dialog.use_native_dialog = true
		file_dialog.file_selected.connect(func(file_path): save_config.emit(file_path))
		file_dialog.file_mode = FileDialog.FILE_MODE_SAVE_FILE
		file_dialog.show()
	elif id == 1:
		var file_dialog = FileDialog.new()
		file_dialog.add_filter("*.json", "JSON")
		file_dialog.use_native_dialog = true
		file_dialog.file_selected.connect(func(file_path): load_config.emit(file_path))
		file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
		file_dialog.show()

func register_camera(camera: Camera2D, name: String) -> void:
	cameras.push_back(camera)
	camera_select.add_item(name)
	
	if camera.is_current():
		camera_select.select(cameras.size() - 1)

func register_agent(agent: Agent, name: String) -> void:
	agents.push_back(agent)
	register_camera(agent.camera, name)
	
func unregister_agent(agent: Agent) -> void:
	var agent_index = agents.find(agent)
	agents.remove_at(agent_index)
	# 0 is default camera
	camera_select.remove_item(agent_index+1)
	if camera_select.selected == -1:
		camera_select.select(0)

func _on_camera_select_btn_item_selected(index: int) -> void:
	cameras[index].make_current()


func _on_solve_btn_pressed() -> void:
	print("Solve!")
	solve_click.emit()

func select_map(map_path: String) -> void:
	map_selector.select_by_name(map_selector.get_map_name(map_path))

func _on_map_selector_map_selected(map_file: String) -> void:
	map_selected.emit(map_file)


func _on_show_presets_toggle_btn_toggled(toggled_on: bool) -> void:
	show_presets_changed.emit(toggled_on)

func preset_list_visible_btn_toggled() -> bool:
	return show_presets_toggle_btn.button_pressed
