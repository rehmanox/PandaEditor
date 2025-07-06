#include <string>
#include <unordered_map>

#include <clockObject.h>
#include <asyncTask.h>
#include <lpoint3.h>
#include <nodePath.h>
#include <texturePool.h>
#include <collisionTraverser.h>

#include "runtimeScript.hpp"
#include "CharacterController.cpp"
#include "ClassicCam.cpp"
#include "ThirdPersonCam.cpp"
#include "CameraCollisionHandler.cpp"
#include "pathUtils.hpp"

const std::string environment_path = PathUtils::to_os_specific("models/Level.egg");
const std::string ralph_path       = PathUtils::to_os_specific("models/ralph.egg.pz");
const std::string ralph_anims_path = PathUtils::to_os_specific("models/ralph-run.egg.pz");

enum CamType {
    Classic,
    ThirdPerson,
    RTS
};

class RoamingRalphDemo : public RuntimeScript {
public:
    RoamingRalphDemo(Demon& demon) :
        RuntimeScript(demon),
        classic_cam(game.main_cam, ralph),
        third_person_cam(game.main_cam, ralph, mouse),
        camera_collision_handler(ralph, c_trav),
        character_controller(ralph, c_trav) {	

        // load environment
        environment = resource_manager.load_model(environment_path);
        environment.reparent_to(game.render);
        environment.set_pos(LPoint3(0.0f, 0.0f, 0.0f));
		
        // ------------------------------------------------------------------------------ //
        // --------------------------- Setup Character Controller ----------------------- //
        // ------------------------------------------------------------------------------ //
        ralph = resource_manager.load_model(ralph_path);
        ralph.reparent_to(game.render);
        ralph.set_scale(0.5f);
        
        std::vector<NodePath> anims = { resource_manager.load_model(ralph_anims_path) };
        LPoint3 start_pos = environment.find("**/Start_Pos").get_pos();
        character_controller.init(anims, start_pos);

        // ------------------------------------------------------------------------------ //
        // ---------------------------- Setup Camera Controller ------------------------ //
        // ------------------------------------------------------------------------------ //
        // Initialize
        classic_cam.init();
        third_person_cam.init();
        camera_collision_handler.init();
        
        // Create a key map and register keys to their corresponding events
        register_keys();

        // Finalize
        // Update at least once before the first 'RoamingRalphDemoUpdate' task update        
        c_trav.traverse(game.render);
        character_controller.update(dt, game.render, input_map);
        update_cam();
    }

protected:
    void on_update(const PT(AsyncTask)&)
    {
        c_trav.traverse(game.render);
        character_controller.update(dt, game.render, input_map);
        update_cam();
    }
    
    void update_cam()
    {
        switch (cam_type)
        {
            case Classic:
                classic_cam.update(dt, input_map);
                break;
            case ThirdPerson:
                third_person_cam.update(dt);
                break;
            case RTS:
                break;
            default:
                classic_cam.update(dt, input_map);;
        }
    }

	void on_event(const std::string& event_name)
    {
		RuntimeScript::on_event(event_name);
    }

private:
	// Character and camera controller related classes
    CharacterController    character_controller;
    ClassicCam             classic_cam;
    ThirdPersonCam         third_person_cam;
    CameraCollisionHandler camera_collision_handler;

    // Environment and character models
    NodePath environment;
    NodePath ralph;
        
    // Global
    CollisionTraverser c_trav;
    
    // Other
    enum CamType cam_type = ThirdPerson;
        
    void register_keys()
    {
        std::unordered_map<std::string, std::pair<std::string, bool>> buttons_map;
        
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
