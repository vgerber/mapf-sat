class_name Agent
extends MAPFAgent

signal pressed

@onready var camera = $Body/Camera
@onready var body = $Body as Sprite2D


var path_mesh: MeshInstance2D = null
var cell_size = Vector2i(125, 125)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	_on_renamed()

func _input(event: InputEvent) -> void:
	pass

func get_camera() -> Camera2D:
	return camera


func _on_collision_area_input_event(viewport: Node, event: InputEvent, shape_idx: int) -> void:
	if event is not InputEventMouseButton:
		return
		
	if event.pressed:
		return
	pressed.emit()


func _on_renamed() -> void:
	$Body/NameLbl.text = name
