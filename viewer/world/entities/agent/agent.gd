class_name Agent
extends MAPFAgent

signal pressed

@onready var camera = $Camera
@onready var body_sprite = $Body

@export var color = Color(1, 1, 1)

var path_mesh: MeshInstance2D = null
var cell_size = Vector2i(125, 125)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	body_sprite.modulate = color

func _input(event: InputEvent) -> void:
	pass
	

# Called every frame. 'delta' is the elapsed time since the previous frame.


func get_camera() -> Camera2D:
	return camera


func _on_collision_area_input_event(viewport: Node, event: InputEvent, shape_idx: int) -> void:
	if event is not InputEventMouseButton:
		return
		
	if event.pressed:
		return
	pressed.emit()
