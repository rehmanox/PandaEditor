#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <array>

// Core/Utility headers
#include <clockObject.h>
#include <trueClock.h>
#include <eventQueue.h>
#include <eventHandler.h>
// Graphics System headers
#include <windowProperties.h>
#include <frameBufferProperties.h>
#include <graphicsPipe.h>
#include <graphicsPipeSelection.h>
#include <graphicsEngine.h>
// camera and lenses
#include <camera.h>
#include <orthographicLens.h>
// Input and Event-Handling headers
#include <mouseWatcher.h>
#include <mouseAndKeyboard.h>
#include <keyboardButton.h>
#include <buttonThrower.h>
// Scene Graph and Rendering headers
#include <nodePath.h>
#include <pgTop.h>
#include <dataGraphTraverser.h>
#include <dataNodeTransmit.h>
// Asynchronous Task Management
#include <asyncTaskManager.h>
#include <genericAsyncTask.h>
// other
#include "exportMacros.hpp"
#include "sceneCam.hpp"
#include "axisGrid.hpp"
#include "resourceManager.hpp"
#include "mouse.hpp"

class ENGINE_API Engine {
public:
    struct EventKey {
        std::string owner;
        std::string name;

        bool operator==(const EventKey& other) const {
            return owner == other.owner && name == other.name;
        }

        // Nested hash function struct
        struct Hash {
            std::size_t operator()(const EventKey& k) const {
                std::size_t h1 = std::hash<std::string>{}(k.owner);
                std::size_t h2 = std::hash<std::string>{}(k.name);
                return h1 ^ (h2 << 1);
            }
        };
    };

    Engine();
    ~Engine();

    // fields
	PT(GraphicsPipe)      pipe;
    PT(GraphicsEngine)    engine;
    PT(GraphicsWindow)    win;
    PT(DisplayRegion)     dr;
    PT(DisplayRegion)     dr2D;

    PT(MouseWatcher)      mouse_watcher;
	
    std::vector<NodePath> mouse_watchers;
	std::vector<NodePath> button_throwers;

    NodePath              data_root;
    DataGraphTraverser    data_graph_trav;
    EventQueue*           event_queue;
    EventHandler*         event_handler;

    NodePath              render;
    SceneCam              scene_cam;
    NodePath              render2D;
    NodePath              aspect2D;
        
    NodePath              pixel2D;
    NodePath              cam2D;

    Mouse                 mouse;
    ResourceManager       resource_manager;
    AxisGrid              axis_grid;
	
    std::unordered_map<EventKey, std::function<void()>, EventKey::Hash> event_handlers;
    std::unordered_map<std::string, std::function<void(const std::string&)>> event_listeners;

    bool should_repaint;
	
    // methods
    void clean_up();
    void update();
    
    void accept(const std::string& event_name, std::function<void()> callback);
    void accept(
        const std::string& owner,
        const std::string& event_name,
        std::function<void()> callback);
    void ignore(const std::string&);
    void ignore(const std::string&, const std::string&);
    void ignore_all();
    void dispatch_event(const char*);
    void trigger(const char*);
    bool has_event(const std::string&);
    bool has_event(const std::string&, const std::string&);
    
    void add_event_listener(const std::string&, std::function<void(std::string)>);
    void remove_event_listener(const std::string&);
    void clear_event_listeners();
 
	void dispatch_events(bool ignore_mouse = false);
    void on_evt_size();
    void show_axis_grid(bool show = false);

	float get_aspect_ratio();
    LVecBase2i get_size();
 
private:
    void create_win();
    void create_3d_render();
    void create_2d_render();
    void create_default_scene();
    void create_axis_grid();
    void setup_mouse_keyboard(PT(MouseWatcher)& mw);
	void process_events(CPT_Event event);
    void reset_clock();
		
	// cache
	std::vector<std::pair<CPT_Event, std::vector<void*>>> panda_events;
    LVecBase2i window_size;
    float aspect_ratio;
};

#endif