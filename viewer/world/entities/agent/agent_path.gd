class_name AgentPath
extends Node2D

@onready var path_mesh = $PathMesh

const cell_size = Vector2i(125, 125)

var assigned_agent: MAPFAgent = null:
	set(agent):
		assigned_agent = agent
		assigned_agent.path_changed.connect(on_path_update)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	position += 0.5 * cell_size
	
	var material = StandardMaterial3D.new()
	
	path_mesh.material = material
	material.albedo_color = Color(1.0, 1.0, 1.0)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func on_path_update() -> void:
	print("Path changed %s" % assigned_agent.get_path_length())
	if assigned_agent.get_path_length() < 2:
		return
		
	var surface_tool = SurfaceTool.new()
	surface_tool.begin(Mesh.PRIMITIVE_LINE_STRIP)
	for path_index in range(assigned_agent.get_path_length()):
		var path_position = assigned_agent.get_path_cell(path_index) * cell_size 
		surface_tool.set_color(Color(1.0, 1.0, 1.0))
		surface_tool.add_vertex(Vector3(path_position.x, path_position.y, 0))
	
	path_mesh.mesh = surface_tool.commit()
