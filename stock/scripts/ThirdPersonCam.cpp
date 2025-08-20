#include <nodePath.h>
#include <lPoint3.h>
#include "mathUtils.hpp"
#include "mouse.hpp"

class ThirdPersonCam {
public:
	// Position settings
    LVecBase3f target_pos_offset = LVecBase3f(0, 0, 5);
    
    int zoom_smooth = 100;
    int min_zoom = 10;
    int max_zoom = 100;
    
    float distance_to_target = 30.0f;
	
	// Orbit settings
    int orbit_smooth = 150;
    float max_rotation_x = 30.0f;
    float min_rotation_x = -30.0f;
    bool lock_y_orbit = false;

	// Cache
    float x_rotation = 0.0f;
    float y_rotation = 0.0f;
    
    // Constructor
    ThirdPersonCam(NodePath& cam, NodePath& tgt, Mouse& mouse) : 
        camera(cam),
        target(tgt),
		mouse(mouse) { reset(); }

    void init()
    { 
        reset();
        update( ClockObject::get_global_clock()->get_dt() );
    }

    void update(float dt)
    {
        move_to_target();
        orbit(dt);
        look_at_target();
    }
    
    void reset()
    { 
        // Set defaults
        target_pos_offset = LVecBase3f(0, 0, 5);
    
        zoom_smooth = 100;
        min_zoom = 10;
        max_zoom = 100;
        
        distance_to_target = 30.0f;
        
        // Orbit settings
        orbit_smooth = 150;
        max_rotation_x = 30.0f;
        min_rotation_x = -30.0f;
        lock_y_orbit = false;

        // Cache
        x_rotation = 0.0f;
        y_rotation = 0.0f;
    }
    
    void zoom(int zoom, float dt)
    {
        distance_to_target += zoom * zoom_smooth * dt;
        distance_to_target = clamp(distance_to_target, min_zoom, max_zoom);
    }

	void toggle_y_orbit_lock()
    {
		lock_y_orbit = !lock_y_orbit;
	}
	    
private:
	// references
	Mouse&    mouse;
    NodePath& camera;
    NodePath& target;

    void move_to_target()
    {
		// Convert degrees to radians
        float yawRad   = y_rotation * (M_PI / 180.0f);
        float pitchRad = x_rotation * (M_PI / 180.0f);
		
		// Spherical coordinates to Cartesian conversion
        float x = distance_to_target  * std::cos(pitchRad) * std::sin(yawRad);
        float y = -distance_to_target * std::cos(pitchRad) * std::cos(yawRad);
        float z = distance_to_target  * std::sin(pitchRad);

		// Set new camera position
        LPoint3f newPos = target.get_pos() + target_pos_offset + LPoint3f(x, y, z);
        LPoint3f smoothedPos = lerp(camera.get_pos(), newPos, 0.5f);
        camera.set_pos(smoothedPos);
    }
    
    void look_at_target()
    {
        camera.look_at(target.get_pos());
    }
    
    void orbit(float dt)
    {
        float abs_dx = std::abs(mouse.get_dx());  // Absolute horizontal movement
        float abs_dy = std::abs(mouse.get_dy());  // Absolute vertical movement
		
        if (abs_dx > abs_dy && !lock_y_orbit)
		{
            y_rotation += mouse.get_horizontal() * orbit_smooth * dt;
            y_rotation = fmod(y_rotation, 360.0f);
        }
		else if (abs_dy > abs_dx && mouse.is_button_down(MOUSE_THREE))
		{
            x_rotation += mouse.get_vertical() * orbit_smooth * dt;
            x_rotation = clamp(x_rotation, min_rotation_x, max_rotation_x);
        }
    }
};
