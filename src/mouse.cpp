#include <algorithm>
#include <iostream>

#include <mouseButton.h>
#include <keyboardButton.h>

#include "taskUtils.hpp"
#include "constants.hpp"
#include "engine.hpp"
#include "mouse.hpp"

const int MOUSE_ONE   = MouseButton::one().get_index();
const int MOUSE_TWO   = MouseButton::two().get_index();
const int MOUSE_THREE = MouseButton::three().get_index();
const int MOUSE_FOUR  = MouseButton::four().get_index();
const int MOUSE_FIVE  = MouseButton::five().get_index();

Mouse::Mouse() :
    _x(0) , _y(0),
    _mx(0), _my(0),
    _dx(0), _dy(0),
    _zoom(0),
    _vertical_axis(0), _horizontal_axis(0),
    _force_relative_mode(false) {}

void Mouse::initialize(WPT(GraphicsWindow) win, WPT(MouseWatcher) mw) {
    _win = win;
    _mouse_watcher = mw;
        
    // Initialize mouse button states
    _mouse_buttons[MOUSE_ONE]   = false;
    _mouse_buttons[MOUSE_TWO]   = false;
    _mouse_buttons[MOUSE_THREE] = false;
    _mouse_buttons[MOUSE_FOUR]  = false;
    _mouse_buttons[MOUSE_FIVE]  = false;
}

void Mouse::update() {
    if (!_mouse_watcher->has_mouse())
        return;

    for (auto& btn : _mouse_buttons) {
        _mouse_buttons[btn.first] = _mouse_watcher->is_button_down(btn.first);
    }

    // Get pointer from screen
    const MouseData pointer_data = _win->get_pointer(0);

    // Delta calculation (new - old)
    _dx = pointer_data.get_x() - _x;
    _dy = pointer_data.get_y() - _y;

    // Normalized coords from watcher
    _mx = _mouse_watcher->get_mouse_x();
    _my = _mouse_watcher->get_mouse_y();

    if (_force_relative_mode) { 
        _horizontal_axis = (_mx > 0) ? 1 : (_mx < 0) ? -1 : 0;
        _vertical_axis   = (_my > 0) ? 1 : (_my < 0) ? -1 : 0;
    } else {
        _horizontal_axis = (_dx > 0) ? 1 : (_dx < 0) ? -1 : 0;
        _vertical_axis   = (_dy > 0) ? 1 : (_dy < 0) ? -1 : 0;
    }

    _x = pointer_data.get_x();
    _y = pointer_data.get_y();

    if (_force_relative_mode) {
        center_mouse();
    }
}

void Mouse::center_mouse() {
    _win->move_pointer(
        0,
        static_cast<int>(_win->get_properties().get_x_size() / 2),
        static_cast<int>(_win->get_properties().get_y_size() / 2));
}

void Mouse::toggle_force_relative_mode() {
    _force_relative_mode = !_force_relative_mode;
    std::cout << "Toggle force mouse relative mode: " << _force_relative_mode << std::endl;
}

bool Mouse::has_mouse() const {
    return _mouse_watcher->has_mouse();
}

bool Mouse::is_button_down(int btn_idx) const {
    auto it = _mouse_buttons.find(btn_idx);
    return (it != _mouse_buttons.end()) ? it->second : false;
}

float Mouse::get_x() const { return _x; }
float Mouse::get_y() const { return _y; }

float Mouse::get_mx() const { return _mx; } // normalized x
float Mouse::get_my() const { return _my; } // normalized y

float Mouse::get_dx() const { return _dx; } // displacement x
float Mouse::get_dy() const { return _dy; } // displacement y

float Mouse::get_vertical() const { return _vertical_axis; }
float Mouse::get_horizontal() const { return _horizontal_axis; }

bool Mouse::is_mouse_centered() const {
    return _force_relative_mode;
}

const std::unordered_map<int, bool>& Mouse::get_mouse_buttons() const { 
    return _mouse_buttons; 
}
