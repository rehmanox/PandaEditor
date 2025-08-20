#include <unordered_map>

#include <nodePath.h>
#include <animControlCollection.h>
#include <collisionTraverser.h>
#include <collisionHandlerQueue.h>
#include <collisionRay.h>
#include <collisionNode.h>
#include <collideMask.h>

#include "demon.hpp"
#include "animUtils.hpp"

class CharacterController {
public:
    CharacterController(NodePath& character, CollisionTraverser& c_trav)
	  : character(character),
        c_trav(c_trav),
        is_moving(false) {}
    
	void init(const std::vector<NodePath>& anims, const LPoint3& start_pos)
	{
        // Initial Pos
        character.set_pos(start_pos);
        
		// Bind the animation to the character
		for (const NodePath& anim : anims) {
			AnimUtils::bind_anims(character, anim, animator);
		}
        
        // Setup collisions 
        this->start_pos = start_pos;
        
        collision_ray = new CollisionRay();
        collision_ray->set_origin(0, 0, 1.5);
        collision_ray->set_direction(0, 0, -1);

        collision_ray_node = new CollisionNode("CharacterCollisionRay");
        collision_ray_node->add_solid(collision_ray);
        collision_ray_node->set_from_collide_mask(CollideMask::bit(0));
        collision_ray_node->set_into_collide_mask(CollideMask::all_off());

        collision_np = character.attach_new_node(collision_ray_node);
        collision_handler_queue = new CollisionHandlerQueue();
        c_trav.add_collider(collision_np, collision_handler_queue);
	}
	
    void update(
        float dt,
        const NodePath& render,
        const std::unordered_map<std::string, bool>& key_map)
	{
        this->update_movement(dt, key_map);
        this->update_collision(render);
	}
    
    void update_movement(float dt, const std::unordered_map<std::string, bool>& key_map)
    {
        if (key_map.at("left"))
			character.set_h(character.get_h() + 300 * dt);
		
		if (key_map.at("right"))
			character.set_h(character.get_h() - 300 * dt);
		
		if (key_map.at("forward"))
			character.set_y(character, -25 * dt);

		bool moving = key_map.at("forward") || key_map.at("left") || key_map.at("right");

		if (moving)
		{
			if (!is_moving)
			{
				animator.get_anim(0)->loop(true);
				is_moving = true;
			}
		}
		else
		{
			if (is_moving)
			{
				animator.stop_all();
				animator.pose("walk", 5);
				is_moving = false;
			}
		}
    }
    
    void update_collision(const NodePath& render)
    {
        int numEntries = collision_handler_queue->get_num_entries();
        if (numEntries == 0) {
            character.set_pos(start_pos);
            return;
        }

        // Sort the collision entries directly using Panda3D's built-in function
        collision_handler_queue->sort_entries();

        // Use the first entry
        CollisionEntry* entry = collision_handler_queue->get_entry(0);

        if (entry->get_into_node()->get_name() == "Collider") {
            character.set_z(entry->get_surface_point(render).get_z());
        }
    }

private:
    NodePath&                 character;
    AnimControlCollection     animator;
	bool                      is_moving;
    LPoint3                   start_pos;
    
    // collision setup    
    PT(CollisionRay)          collision_ray;
    PT(CollisionNode)         collision_ray_node;
    NodePath                  collision_np;
    PT(CollisionHandlerQueue) collision_handler_queue;
    CollisionTraverser&       c_trav;
};
