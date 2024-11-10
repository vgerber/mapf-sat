extends PanelContainer

signal add_preset(preset_name: String)
signal load_preset(preset: MAPFConfig.ConfigPreset)
signal remove_preset(preset: MAPFConfig.ConfigPreset)


@onready var preset_name_txt = $Margin/VBox/AddContainer/PresetNameTxt
@onready var preset_list = $Margin/VBox/PresetList

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func clear_preset_list() -> void:
	while preset_list.get_child_count() > 0:
		preset_list.remove_child(preset_list.get_child(0))

func add_preset_to_list(preset: MAPFConfig.ConfigPreset) -> void:
	var label = Label.new()
	label.size_flags_horizontal = Control.SIZE_EXPAND
	label.text = preset.name
	preset_list.add_child(label)
	
	var remove_btn = Button.new()
	remove_btn.text = "Remove"
	remove_btn.pressed.connect(func(): remove_preset.emit(preset))
	preset_list.add_child(remove_btn)
	
	var load_btn = Button.new()
	load_btn.text = "Load"
	load_btn.pressed.connect(func(): load_preset.emit(preset))
	preset_list.add_child(load_btn)
	
	

func _on_add_preset_btn_pressed() -> void:
	var text: String = preset_name_txt.text
	text = text.replace(" ", "")
	if text.length() == 0:
		return
	print("Adding preset " + text)
	add_preset.emit(text)
