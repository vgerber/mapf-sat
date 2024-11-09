class_name MapSelector
extends HBoxContainer

signal map_selected(map_file: String)

var maps: Array[String] = []
@onready var option_control = $MapOptionBtn

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	maps = load_all_maps("res://assets/maps")
	var map_names = maps.map(get_map_name)
	map_names.sort()
	for map_name in map_names:
		option_control.add_item(map_name)	


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func get_map_name(map_file_path: String) -> String:
	var map_name_parts = map_file_path.split("/")
	return map_name_parts [map_name_parts.size()-1].replace(".map","")

func load_all_maps(path: String) -> Array[String]:
	var map_files: Array[String] = []
	
	var maps_dir = DirAccess.open(path)
	if not maps_dir:
		printerr("Failed to opens maps")
		return map_files
	maps_dir.list_dir_begin()
	var file_name = maps_dir.get_next()
	while file_name != "":
		if maps_dir.current_is_dir():
			map_files += load_all_maps("%s/%s" % [path, file_name])
		else:
			map_files.push_back("%s/%s" % [path, file_name])
		file_name = maps_dir.get_next()
	return map_files


func _on_map_option_btn_item_selected(index: int) -> void:
	map_selected.emit(maps[index])

static func get_map_absolute_path(map_path: String) -> String:
	if OS.has_feature("editor"):
		return ProjectSettings.globalize_path(map_path)
	else:
		return OS.get_executable_path().get_base_dir().path_join(map_path.replace("res://", ""))
