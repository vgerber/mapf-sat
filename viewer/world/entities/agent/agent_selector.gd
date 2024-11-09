class_name AgentSelector
extends Sprite2D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func assign_agent(agent: Agent) -> void:
	if agent == null:
		hide()
		return
	
	
	var parent = get_parent()
	if parent == null:
		agent.body.add_child(self)
	else:
		reparent(agent.body, false)
	modulate = agent.body.modulate
	show()
