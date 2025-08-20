#include <nodePath.h>
#include <unordered_map>
#include <iostream>

class ClassicCam {
public:
    LVector3f look_pos_offset;
    float min_distance;
    float max_distance;
    float speed;

    ClassicCam(NodePath& cam_np, NodePath& target_np) : 
        camera(cam_np),
        target_np(target_np)
    {
        look_pos_offset = LVector3(0.f, 0.f, 1.5f);
        min_distance = 5.f;
        max_distance = 20.f;
        speed = 20.f;
    }

    void init() {
        init(max_distance, 12.5f);
    }

    void init(float distance_to_target, float height) {
        look_target = NodePath("CamLookAtTarget");
        look_target.reparent_to(target_np);
        look_target.set_pos(look_pos_offset);

        camera.set_pos(target_np.get_x(), target_np.get_y() - distance_to_target, height);
        
        update_movement();
    }

    void update(float dt, const std::unordered_map<std::string, bool>& input_map) {        
        if (input_map.at("cam-left"))
            camera.set_x(camera, -speed * dt);
        else if (input_map.at("cam-right"))
            camera.set_x(camera, speed * dt);

        update_movement();
    }

protected:
    void update_movement() {
        LVector3 cam_vec = target_np.get_pos() - camera.get_pos();
        cam_vec.set_z(0);

        float cam_dist = cam_vec.length();
        cam_vec.normalize();

        if (cam_dist > max_distance)
        {
            camera.set_pos(camera.get_pos() + cam_vec * (cam_dist - max_distance));
            cam_dist = max_distance;
        } 
        else if (cam_dist < min_distance)
        {
            camera.set_pos(camera.get_pos() - cam_vec * (min_distance - cam_dist));
            cam_dist = min_distance;
        }

        camera.look_at(look_target);
    }

private:
    NodePath& camera;
    NodePath& target_np;
    NodePath look_target;
};
