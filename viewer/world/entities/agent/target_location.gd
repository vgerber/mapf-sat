class_name TargetLocation
extends Node2D

func assign_agent(agent: Agent) -> void:
	$PathTarget.modulate = agent.body.modulate
	agent.target_changed.connect(func(target: Vector2i): _on_agent_target_changed(target, agent.cell_size))
	agent.path_index_changed.connect(func(path_index: int): _on_agent_path_changed(agent))
	agent.path_changed.connect(func(): _on_agent_path_changed(agent))
	$PathTarget/AgentNameLbl.text = agent.name

func _on_agent_target_changed(target: Vector2i, cell_size: Vector2i) -> void:
	position = (Vector2(target) + Vector2(0.5, 0.5)) * Vector2(cell_size)
	show()
	
func _on_agent_path_changed(agent: Agent):
	visible = agent.get_current_path_index() == max(0, agent.get_path_length()-1)
