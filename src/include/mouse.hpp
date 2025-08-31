#ifndef MOUSE_H
#define MOUSE_H

#include <unordered_map>
#include <vector>
#include <string>

#include "exportMacros.hpp"

class MouseWatcher;
class GraphicsWindow;

extern ENGINE_API const int MOUSE_ONE;
extern ENGINE_API const int MOUSE_TWO;
extern ENGINE_API const int MOUSE_THREE;
extern ENGINE_API const int MOUSE_FOUR;
extern ENGINE_API const int MOUSE_FIVE;

class ENGINE_API Mouse {
public:
    Mouse();

	void initialize(WPT(GraphicsWindow), WPT(MouseWatcher));
    void update();
    
	void center_mouse();
	void toggle_force_relative_mode();
        
    void clear_modifier(int index);
    bool has_modifier(int modifier) const;
    
    bool has_mouse() const;
	bool is_button_down(int btn_idx) const;
	bool is_mouse_centered() const;
    
    // Getters
	float get_x()  const;
    float get_y()  const;
    
    float get_mx() const;
    float get_my() const;
    
    float get_dx() const;
    float get_dy() const;
    
	int   get_zoom() const;
	
	float get_vertical()   const;
	float get_horizontal() const;

	const std::unordered_map<int, bool>& get_mouse_buttons() const;

private:
    // raw mouse pointer x and y (pixel co-ordinates)
    float _x;
    float _y;
	
    // mouse x and y (screen co-ordinates)
    float _mx;
    float _my;

    // mouse dx and dy (movement displacement from last frame)
	float _dx;
    float _dy;
    
	int _zoom;
	
	float _vertical_axis;
	float _horizontal_axis;

	bool _force_relative_mode;
	
    WPT(GraphicsWindow) _win;
    WPT(MouseWatcher)   _mouse_watcher;
    
    std::unordered_map<int, bool> _mouse_buttons;
};

#endif // MOUSE_H
