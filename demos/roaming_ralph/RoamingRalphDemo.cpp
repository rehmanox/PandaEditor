#include <string>
#include <unordered_map>

#include <clockObject.h>
#include <asyncTask.h>
#include <lpoint3.h>
#include <nodePath.h>
#include <texturePool.h>
#include <collisionRay.h>
#include <collisionNode.h>
#include <collisionTraverser.h>
#include <collisionHandlerQueue.h>
#include <collideMask.h>

#include "runtimeScript.hpp"
#include "CharacterController.cpp"
#include "CharacterCollisionHandler.cpp"
#include "CameraController.cpp"
#include "CameraCollisionHandler.cpp"

const std::string environment_path = "Level.egg";
const std::string ralph_path       = "ralph.egg.pz";
const std::string ralph_anims_path = "ralph-run.egg.pz";

class RoamingRalphDemo : public RuntimeScript {
public:
    RoamingRalphDemo(Demon& demon) :
        RuntimeScript(demon),
        camera_controller(ralph, game.main_cam),
        camera_collision_handler(ralph, c_trav),
        character_controller(ralph),
        character_collision_handler(ralph, c_trav) {		

        // load stuff
        load_world();
        load_actor();
		
        std::vector<NodePath> anims = load_actor_anims();
        LPoint3 start_pos = environment.find("**/Start_Pos").get_pos();

        // Take ralph to the starting position
        ralph.set_pos(start_pos);

        // Initialize
        character_controller.init(anims);
        character_collision_handler.init(start_pos);
        camera_controller.init();
        camera_collision_handler.init();
        
        // Create a key map and register keys to their corresponding events
        register_keys();

        // Finalize
        // Update at least once before the first 'RoamingRalphDemoUpdate' task update        
        c_trav.traverse(game.render);
        character_controller.update(dt, input_map);
        character_collision_handler.update(game.render);
        camera_controller.update(dt, input_map);
    }

protected:
    void on_update(const PT(AsyncTask)&) {
        c_trav.traverse(game.render);
        character_controller.update(dt, input_map);
        character_collision_handler.update(game.render);
        camera_controller.update(dt, input_map);
    }

	void on_event(const std::string& event_name)
    {
		RuntimeScript::on_event(event_name);
    }

private:
	// Character and camera controller related classes
    CharacterController       character_controller;
    CharacterCollisionHandler character_collision_handler;
    CameraController          camera_controller;
    CameraCollisionHandler    camera_collision_handler;

    // Environment and character models
    NodePath environment;
    NodePath ralph;
        
    // Global
    CollisionTraverser c_trav;
    
    // Input handling
    std::unordered_map<std::string, std::pair<std::string, bool>> buttons_map;

	
    void load_world()
    {
        environment = resource_manager.load_model(environment_path);
        environment.reparent_to(game.render);
        environment.set_pos(LPoint3(0.0f, 0.0f, 0.0f));
    }

    void load_actor()
    {
        // load character model
        ralph = resource_manager.load_model(ralph_path);
        ralph.reparent_to(game.render);
        ralph.set_scale(0.5f);
    }
    
    std::vector<NodePath> load_actor_anims() const
    {
        NodePath walk_anim = resource_manager.load_model(ralph_anims_path);
        return {walk_anim};
    }

    void register_keys()
    {
        buttons_map["a"] = {"left",      true};
        buttons_map["d"] = {"right",     true};
        buttons_map["w"] = {"forward",   true};
        buttons_map["e"] = {"cam-left",  true};
        buttons_map["q"] = {"cam-right", true};

        buttons_map["a-up"] = {"left",      false};
        buttons_map["d-up"] = {"right",     false};
        buttons_map["w-up"] = {"forward",   false};
        buttons_map["e-up"] = {"cam-left",  false};
        buttons_map["q-up"] = {"cam-right", false};
		
		// register_button_map is defined in base RuntimeScript class
		this->register_button_map(buttons_map);
    }
};
