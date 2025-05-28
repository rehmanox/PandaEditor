#ifndef DEMON_H
#define DEMON_H

#include <unordered_map>
#include <vector>
#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "engine.hpp"
#include "game.hpp"
#include "p3d_Imgui.hpp"
#include "dllLoader.hpp"

class Demon {
public:
	struct Settings {
		GameViewStyle game_view_style;
		float game_view_size;
	};
      
    // Delete copy constructor and assignment operator
	// necessary for singleton
    Demon(const Demon&) = delete;
    Demon& operator=(const Demon&) = delete;
	
    static Demon& get_instance();

	// Methods
	void start();
	void bind_events();
	void unbind_events();
	void enable_game_mode();
	void exit_game_mode();
	bool is_game_mode();
	void increase_game_view_size();
	void decrease_game_view_size();
	void update_game_view();
	void update_game_view(GameViewStyle style);
	void update_game_view(GameViewStyle style, float width,  float height);
	void exit();
	
	// Fields
	Engine engine;
	Game game;
	Settings settings = {GameViewStyle::BOTTOM_LEFT, 0.3f};
	Settings default_settings = {GameViewStyle::BOTTOM_LEFT, 0.3f};
    PT(MouseWatcherRegion) game_mw_region;
    
    // ImGUI instance
    Panda3DImGui p3d_imgui;
    
private:
    Demon();
    ~Demon();
    
    std::unordered_map<std::string, std::string> config;
    DllLoader dllLoader;

	// Methods
    void load_config(const std::string& filepath);
    std::vector<std::string> load_dll_functions(const std::string& filepath);
	void setup_paths();
	
	// ImGui fields and methods
	void init_imgui(Panda3DImGui *panda3d_imgui, NodePath *parent, MouseWatcher* mw, std::string name);
	void imgui_update();
	
	// Fields
    std::string dll_functions_file;
    
    bool _is_started;
	bool _cleaned_up;
	bool _game_mode_enabled;
	bool _mouse_over_ui;
	int  _num_frames_since_last_repait;
    
	// Delete the 'delete' operator to prevent manual deletion
	// necessary for singleton
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;  // Prevents array deletion
};

#endif
