class_name MAPFConfig
extends Object

var presets: Array[ConfigPreset]

class ConfigAgent:
	var x: int
	var y: int
	var target_x: int
	var target_y: int
	
	func _init(x: int, y: int, target_x: int, target_y: int) -> void:
		self.x = x
		self.y = y
		self.target_x = target_x
		self.target_y = target_y
	
	func to_dict() -> Dictionary:
		return {
			"x": x,
			"y": y,
			"target_x": target_x,
			"target_y": target_y,
		}
		
	static func from_dict(dict: Dictionary) -> ConfigAgent:
		return ConfigAgent.new(dict["x"], dict["y"], dict["target_x"], dict["target_y"])
	
	static func from_agent(agent: Agent) -> ConfigAgent:
		return ConfigAgent.new(agent.cell.x, agent.cell.y, agent.target_cell.x, agent.target_cell.y)

class ConfigPreset:
	var name: String
	var map_path: String
	var agents: Array[ConfigAgent]
	
	func _init(name: String, map_path: String, agents: Array[ConfigAgent]) -> void:
		self.name = name
		self.map_path = map_path
		self.agents = agents
	
	func to_dict() -> Dictionary:
		return {
			"name": name,
			"map_path": map_path,
			"agents": agents.map(func(agent: ConfigAgent): return agent.to_dict())
		}
		
	static func from_dict(dict: Dictionary) -> ConfigPreset:
		var agents: Array[ConfigAgent] = []
		agents.assign(dict["agents"].map(ConfigAgent.from_dict))
		return ConfigPreset.new(dict["name"], dict["map_path"], agents)
	
func _init(presets: Array[ConfigPreset]) -> void:
	self.presets = presets
	
func to_dict() -> Dictionary:
	return {
		"presets": presets.map(func(preset: ConfigPreset): return preset.to_dict())
	}
	
static func from_dict(dict: Dictionary) -> MAPFConfig:
	var presets: Array[ConfigPreset] = []
	presets.assign(dict["presets"].map(ConfigPreset.from_dict))
	return MAPFConfig.new(presets)
