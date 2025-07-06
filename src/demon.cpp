#include <asyncTask.h>
#include <genericAsyncTask.h>
#include <config_putil.h>
#include <nodePath.h>
#include <bitMask.h>

#include "pathUtils.hpp"
#include "taskUtils.hpp"
#include "mathUtils.hpp"
#include "helperUtils.hpp"
#include "demon.hpp"
#include "imgui.h"

Demon& Demon::get_instance() {
    static Demon instance;
    return instance;
}

Demon::Demon() : game(*this) {    
    // Load configuration
    std::string config_file = PathUtils::join_paths(
    PathUtils::get_executable_dir(),
    "game_config.txt");
    load_config(config_file);
    
	// Initializations
	setup_paths();
	init_imgui(&p3d_imgui, &engine.pixel2D, engine.mouse_watcher, "Editor");
	game.init();
		
	// Setup game and editor viewport camera masks
	BitMask32 ed_mask   = BitMask32::bit(0);
	BitMask32 game_mask = BitMask32::bit(1);
	
	DCAST(Camera, engine.scene_cam.node())->set_camera_mask(ed_mask);
	DCAST(Camera, engine.cam2D.node())->set_camera_mask(ed_mask);
	
	DCAST(Camera, game.main_cam.node())->set_camera_mask(game_mask);
	DCAST(Camera, game.cam2D.node())->set_camera_mask(game_mask);
	
    // Define MouseWatcherRegion
    // %r	Replaced with the name of the region (i.e. region.get_name())
    // %w	Replaced with the name of the MouseWatcher generating the event
	float size = default_settings.game_view_size;	
	game_mw_region = new MouseWatcherRegion("GameMWRegion", 0.f, size, 0.f, size);
    engine.mouse_watcher->set_enter_pattern("enter-%r");
    engine.mouse_watcher->set_leave_pattern("leave-%r");
    engine.mouse_watcher->set_within_pattern("within-%r");
	engine.mouse_watcher->add_region(game_mw_region);
    
	// Hide editor only geo from game view and vice versa
	engine.axis_grid.hide(game_mask);
	engine.render2D.find("**/SceneCameraAxes").hide(game_mask);
	p3d_imgui.get_root().hide(game_mask);

	// Create update task
	PT(AsyncTask) update_task =
        (make_task([this](AsyncTask *task) -> AsyncTask::DoneStatus {

		engine.update();		
		imgui_update();
		engine.dispatch_events(_mouse_over_ui);
		engine.engine->render_frame();

		_mouse_over_ui = false;
		
		if(engine.should_repaint) {
			p3d_imgui.should_repaint = true;
			
			if(_num_frames_since_last_repait > 2) {
				
				engine.should_repaint = false;
				_num_frames_since_last_repait = 0;
			}
			_num_frames_since_last_repait++;
		}
		
		return AsyncTask::DS_cont;
	}, "EngineUpdate"));
	
	update_task->set_sort(MAIN_TASK_SORT);
	AsyncTaskManager::get_global_ptr()->add(update_task);
	
    // Loop through all tasks in the task manager
    auto task_mgr = AsyncTaskManager::get_global_ptr();
    AsyncTaskCollection tasks = task_mgr->get_tasks();
    for (int i = 0; i < tasks.get_num_tasks(); ++i) {
        PT(AsyncTask) task = tasks.get_task(i);
        std::cout << "Task " << i + 1 << ": " << task->get_name() << std::endl;
    }
	
	// Bind events
	bind_events();
	
	// Set defaults
	float game_view_size = default_settings.game_view_size;
	update_game_view(GameViewStyle::BOTTOM_LEFT, game_view_size, game_view_size);
	
	engine.mouse.set_mouse_mode(WindowProperties::M_absolute);
	
    // Get DLL functions file
    dll_functions_file = PathUtils::join_paths(
        PathUtils::get_executable_dir(),
        "export_functions.txt");
    
    // Load editor DLLs
    const std::vector<std::string> dll_functions = load_dll_functions(dll_functions_file);
    std::vector<std::string> ed_functions;

    const std::string prefix = "create_instance_Editor_";
    std::copy_if(
        dll_functions.begin(), dll_functions.end(),
        std::back_inserter(ed_functions),
        [&](const std::string& name) {
            return name.size() >= prefix.size() &&
               name.compare(0, prefix.size(), prefix) == 0;
        });

    dllLoader.load_script_dll(ed_functions, "game_script.dll", *this);
    
    // Add event hooks
	engine.accept("window-event", [this]() { engine.on_evt_size(); } );
    
    // Others
	_cleaned_up        = false;
	_game_mode_enabled = false;
	_mouse_over_ui     = false;
    _is_started        = false;
}

Demon::~Demon() { 
    if (_game_mode_enabled) {
        exit_game_mode();

        PT(AsyncTask) exit_task =
            (make_task([this](AsyncTask *task) -> AsyncTask::DoneStatus {
            unbind_events();
            exit(); 
            return AsyncTask::DS_done;
        }, "ScriptsUnloadTask"));
        
        // Exit game mode also creates a task to unload scripts with delay = 0,
        // we set delay = 1 for exit task so it executes after scripts unload task.
        exit_task->set_delay(1);
        AsyncTaskManager::get_global_ptr()->add(exit_task);
        
    } else {
        unbind_events();
        exit(); 
    }
}

void Demon::start() {
    if (_is_started)
        return;
    
    _is_started = true;
    
    // Call size event handlers to scale
    // renders properly, before first update
    engine.on_evt_size();
    game.on_evt_size();
    
    // Start the update
	while (!engine.win->is_closed()) {
		AsyncTaskManager::get_global_ptr()->poll();	
	}
}

void Demon::load_config(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filepath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        auto sep = line.find(':');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);
            // Trim whitespace
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            config[key] = value;
        }
    }
}

std::vector<std::string> Demon::load_dll_functions(const std::string& filepath) {
    std::vector<std::string> functions;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open function list: " << filepath << std::endl;
        return functions;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (!line.empty()) {
            functions.push_back(line);
        }
    }
    return functions;
}

void Demon::setup_paths() {
    // Shared assets
    std::string shared_assets = config["shared_assets"];
    shared_assets = Filename::from_os_specific(shared_assets);
    
    // Project specific assets
    std::string project_assets = PathUtils::join_paths(
        config["project_dir"],
        "assets");
    project_assets = Filename::from_os_specific(project_assets);

	get_model_path().prepend_directory(Filename::from_os_specific(shared_assets));
	get_model_path().prepend_directory(Filename::from_os_specific(project_assets));
    
    // Developer mode only paths
    std::string dev_assets = PathUtils::join_paths(
        config["working_dir"],
        "assets");
    get_model_path().prepend_directory(Filename::from_os_specific(dev_assets));
}

void Demon::bind_events() {
	engine.accept("shift-1", [this]() { update_game_view(GameViewStyle::BOTTOM_LEFT);  });
	engine.accept("shift-2", [this]() { update_game_view(GameViewStyle::BOTTOM_RIGHT); });
	engine.accept("shift-3", [this]() { update_game_view(GameViewStyle::TOP_LEFT);     });
	engine.accept("shift-4", [this]() { update_game_view(GameViewStyle::TOP_RIGHT);    });
	engine.accept("shift-5", [this]() { update_game_view(GameViewStyle::CENTER);       });
	engine.accept("shift-i", [this]() { increase_game_view_size();                     });
	engine.accept("shift-d", [this]() { decrease_game_view_size();                     });
	
    engine.accept("shift-e", [this]() { exit(); });

    if (!engine.has_event("ENGINE", "shift-g")) {
        engine.accept("shift-g",   [this]() {
            if (!is_game_mode()) {
                enable_game_mode();
            }
            else {
                exit_game_mode();
            }
        });
    }
}

void Demon::unbind_events() {
    /*
    engine.ignore("ENGINE", "shift-1");
	engine.ignore("ENGINE", "shift-2");
	engine.ignore("ENGINE", "shift-3");
	engine.ignore("ENGINE", "shift-4");
	engine.ignore("ENGINE", "shift-5");
	engine.ignore("ENGINE", "shift-i");
	engine.ignore("ENGINE", "shift-d");
    */
}

void Demon::enable_game_mode() {
	if (_game_mode_enabled)
		return;

    engine.ignore("ENGINE", "shift-e");
    
    // load exported functions
    std::vector<std::string> dll_functions = load_dll_functions(dll_functions_file);
    
    // load dlls
    dllLoader.load_script_dll(dll_functions, "game_script.dll", *this);
    
    // Enable game mode
	engine.trigger("game_mode_enabled");
	std::cout << "Game mode enabled\n";
    _game_mode_enabled = true;
}

void Demon::exit_game_mode() {
	if (!_game_mode_enabled)
		return;

	engine.trigger("game_mode_disabled");
    
    // 'exit_game_mode' sends "game_mode_disabled" event signaling
    // user-scripts to stop and clean_up, which may take a frame, so
    // we defer scripts unloading to next epoch.

    // Create task to unload scripts
    PT(AsyncTask) scripts_unload_task =
        (make_task([this](AsyncTask *task) -> AsyncTask::DoneStatus {
        dllLoader.unload_all_scripts();
        return AsyncTask::DS_done;
    }, "ScriptsUnloadTask"));

    scripts_unload_task->set_delay(0);
    AsyncTaskManager::get_global_ptr()->add(scripts_unload_task);
    engine.accept("shift-e", [this]() { exit(); });
    
    // Clear the scene graphs, except for cameras
    remove_children_except(game.render,   { game.main_cam });
    remove_children_except(game.render2D, { game.cam2D, game.aspect2D, game.pixel2D });
    remove_children_except(game.aspect2D, { game.cam2D });
    remove_children_except(game.pixel2D,  { game.cam2D });
    // --------------------------------------------------------------------------------
    
    std::cout << "Game mode disabled\n";
	_game_mode_enabled = false;
}

void Demon::increase_game_view_size() {
	float increment = (1.0f - default_settings.game_view_size) / 4.0f;	
	float min_size = default_settings.game_view_size;
	settings.game_view_size = clamp(settings.game_view_size + increment, min_size, 1.0f);
	this->update_game_view();
}

void Demon::decrease_game_view_size() {	
	float decrement = (1.0f - default_settings.game_view_size) / 4.0f;
	float min_size = default_settings.game_view_size;
	settings.game_view_size = clamp(settings.game_view_size - decrement, min_size, 1.0f);
	this->update_game_view();
}

void Demon::update_game_view() {
	GameViewStyle style = settings.game_view_style;
	float size = settings.game_view_size;	
	this->update_game_view(style, size, size);
}

void Demon::update_game_view(GameViewStyle style) {
	settings.game_view_style = style;
	float size = settings.game_view_size;	
	this->update_game_view(style, size, size);
}

void Demon::update_game_view(GameViewStyle style, float width, float height) {
    float left, right, bottom, top;

    switch (style) {
        case CENTER:
            left   = 0.5f - width / 2;
            right  = 0.5f + width / 2;
            bottom = 0.5f - height / 2;
            top    = 0.5f + height / 2;
            break;

        case BOTTOM_LEFT:
            left   = 0;
            right  = width;
            bottom = 0;
            top    = height;
            break;

        case BOTTOM_RIGHT:
            left   = 1 - width;
            right  = 1;
            bottom = 0;
            top    = height;
            break;

        case TOP_LEFT:
            left   = 0;
            right  = width;
            bottom = 1 - height;
            top    = 1;
            break;

        case TOP_RIGHT:
            left   = 1 - width;
            right  = 1;
            bottom = 1 - height;
            top    = 1;
            break;

        default:
            left   = 0.5f - width / 2;
            right  = 0.5f + width / 2;
            bottom = 0.5f - height / 2;
            top    = 0.5f + height / 2;
            break;
    }

    // Update 3D and 2D Display Regions
    game.dr3D->set_dimensions(left, right, bottom, top);
    game.dr2D->set_dimensions(left, right, bottom, top);

    // Convert from [0,1] range to [-1,1] range for MouseWatcherRegion
    float mw_left   = 2 * left - 1;
    float mw_right  = 2 * right - 1;
    float mw_bottom = 2 * bottom - 1;
    float mw_top    = 2 * top - 1;

    // Update the game MouseWatcherRegion dimensions
    game_mw_region->set_frame(mw_left, mw_right, mw_bottom, mw_top);
}

bool Demon::is_game_mode() {
	return _game_mode_enabled == true;
}

void Demon::exit() {
	if(_cleaned_up)
		return;
    
    p3d_imgui.clean_up();
	engine.clean_up();

	_cleaned_up = true;
}

// ----------------------------------------- imgui integration ----------------------------------------- //
void Demon::init_imgui(
    Panda3DImGui *panda3d_imgui,
    NodePath *parent,
    MouseWatcher* mw,
    std::string name) {

	// Setup ImGUI for Panda3D
	panda3d_imgui->init(engine.win, mw, parent);
	panda3d_imgui->setup_style();
    panda3d_imgui->setup_geom();
    panda3d_imgui->setup_shader(Filename("shaders"));
    panda3d_imgui->setup_font();
    panda3d_imgui->setup_event();
    panda3d_imgui->enable_file_drop();
}

void Demon::imgui_update() {
	ImGui::SetCurrentContext(this->p3d_imgui.context_);
	if (this->p3d_imgui.should_repaint) {
		this->p3d_imgui.on_window_resized();
		this->p3d_imgui.should_repaint = false;
	}
    
	this->p3d_imgui.new_frame_imgui();
    
    // Handle mouse
    MouseWatcher* mw = this->engine.mouse_watcher;
    if(mw->has_mouse()) {
        for (const ButtonHandle& button: this->p3d_imgui.btn_handles) {
            if(mw->is_button_down(button))
                this->p3d_imgui.on_button_down_or_up(button, true);
            else
                this->p3d_imgui.on_button_down_or_up(button, false);
        }
    }
    
    // 
	engine.trigger("render_imgui");
	this->p3d_imgui.render_imgui();
	if(ImGui::GetIO().WantCaptureMouse) { _mouse_over_ui = true; }
}
