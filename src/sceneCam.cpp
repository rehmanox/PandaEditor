#include <lVecBase3.h>
#include <lVecBase2.h>
#include <lQuaternion.h>
#include <camera.h>
#include <perspectiveLens.h>
#include <lineSegs.h>
#include <clockObject.h>

#include "constants.hpp"
#include "mathUtils.hpp"
#include "engine.hpp"
#include "sceneCam.hpp"

SceneCam::SceneCam(Engine& engine, float move_speed, const LVecBase3f& default_pos)
    : engine(engine), move_speed(move_speed), default_pos(default_pos), delta_speed(0.0f) {

    // Create camera
    PT(Camera) cam_node = new Camera("SceneCamera");
    cam_np = NodePath(cam_node);

    // Create and configure lens
    PT(PerspectiveLens) lens = new PerspectiveLens();
    lens->set_fov(60);
    lens->set_aspect_ratio(800.0f / 600.0f);
    cam_node->set_lens(lens);

    // Initialize the NodePath
    NodePath::operator=(cam_np);

    // Create the target
    target = NodePath("TargetNode");
}

NodePath SceneCam::create_axes(float thickness, float length) {
    LineSegs ls;
    ls.set_thickness(thickness);

    // Define axis colors
    struct { float r, g, b; } colors[] = {
        {1.0f, 0.0f, 0.0f}, // X - Red
        {0.0f, 1.0f, 0.0f}, // Y - Green
        {0.0f, 0.0f, 1.0f}  // Z - Blue
    };

    // Draw X, Y, Z axes
    for (int i = 0; i < 3; ++i) {
        ls.set_color(colors[i].r, colors[i].g, colors[i].b, 1.0f);
        ls.move_to(0.0f, 0.0f, 0.0f);
        LVecBase3f dir(length * (i == 0), length * (i == 1), length * (i == 2));
        ls.draw_to(dir);
    }

    return NodePath(ls.create());
}

void SceneCam::initialize() {
    axes = create_axes();
    axes.set_name("SceneCameraAxes");
    axes.reparent_to(engine.pixel2D);
    axes.set_scale(1.5);
}

void SceneCam::move(const LVecBase3f& move_vec) {
    LVecBase3f camera_vec = get_pos() - target.get_pos();
    LVecBase3f modified_move_vec = move_vec * (camera_vec.length() / 300.0f);

    set_pos(*this, modified_move_vec);
    target.set_quat(get_quat());

    LVecBase3f target_move_vec(modified_move_vec.get_x(), 0, modified_move_vec.get_z());
    target.set_pos(target, target_move_vec);
}

void SceneCam::orbit(const LVecBase2f& delta) {
    LVecBase3f hpr = get_hpr();
    hpr.set_x(hpr.get_x() + delta.get_x());
    hpr.set_y(hpr.get_y() + delta.get_y());
    set_hpr(hpr);

    float rad_x = hpr.get_x() * (M_PI / 180.0f);
    float rad_y = hpr.get_y() * (M_PI / 180.0f);
    
    LVecBase3f camera_vec = get_pos() - target.get_pos();
    float cam_vec_dist = camera_vec.length();

    LVecBase3f new_pos = target.get_pos() + LVecBase3f(
        cam_vec_dist * sin(rad_x) * cos(rad_y),
        -cam_vec_dist * cos(rad_x) * cos(rad_y),
        -cam_vec_dist * sin(rad_y)
    );

    set_pos(new_pos);
}

void SceneCam::update() {
    if (!engine.mouse.has_mouse() || !engine.mouse.has_modifier(ALT_KEY_IDX))
        return;

    delta_speed = move_speed * ClockObject::get_global_clock()->get_dt();

    if (engine.mouse.is_button_down(MOUSE_ONE)) {
        orbit(LVecBase2f(engine.mouse.get_dx() * delta_speed, engine.mouse.get_dy() * delta_speed));
    } else if (engine.mouse.is_button_down(MOUSE_TWO)) {
        move(LVecBase3f(engine.mouse.get_dx() * delta_speed, 0, -engine.mouse.get_dy() * delta_speed));
    } else if (engine.mouse.is_button_down(MOUSE_THREE)) {
        move(LVecBase3f(0, -engine.mouse.get_dx() * delta_speed, 0));
    }

    update_axes();
}

void SceneCam::update_axes() {
    axes.set_pos(engine.get_size().get_x()-50, 0, -50);
    LQuaternion camera_quat(get_quat());
    camera_quat.invert_in_place();
    axes.set_quat(camera_quat);
}

void SceneCam::reset() {
    target.set_pos(LVecBase3f(0, 0, 0));
    set_pos(default_pos);
    look_at(target.get_pos());
    target.set_quat(get_quat());
    update_axes();
}

void SceneCam::on_resize_event(float aspect_ratio) {
    auto lens = DCAST(PerspectiveLens, DCAST(Camera, cam_np.node())->get_lens());
    if (lens) {
        lens->set_aspect_ratio(aspect_ratio);
    }
    update_axes();
}
