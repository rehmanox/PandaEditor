#ifndef GAME_H
#define GAME_H

#include "p3d_Imgui.hpp"

class MouseWatcher;
class DisplayRegion;
class ButtonThrower;
class NodePath;
class Demon;

class Game {
public:
    explicit Game(Demon& demon);
    void init();
    void on_evt_size();
    void on_evt(const std::string& event_name); // Pass string by const reference
	
    // game_render and game_render_2D are the actual renders parented to engine.render
    // render and render_2D are meant to meant to meant to nodepaths loaded at runtime 
    // and they will be cleaned as soon as 'level_script' is unloaded.
    NodePath          game_render;
    NodePath          render;
    
    NodePath          game_render_2D;
    NodePath          render2D;
    NodePath          aspect2D;
    NodePath          pixel2D;
    NodePath          main_cam;
    NodePath          cam2D;
    
    PT(DisplayRegion) dr3D;
    PT(DisplayRegion) dr2D;
    PT(MouseWatcher)  mouse_watcher;

private:
    Demon& demon;

    void create_dr2D();
    void create_dr3D();
    void create_mouse_watcher_3D();
};

#endif // GAME_H
