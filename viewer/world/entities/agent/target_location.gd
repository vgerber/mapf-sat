class_name TargetLocation
extends Node2D

@export var color = Color(1,1,1, 0.1)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	$PathTarget.modulate = color


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
