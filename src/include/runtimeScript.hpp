#ifndef RUNTIME_SCRIPT_H
#define RUNTIME_SCRIPT_H

#include <unordered_map>
#include <iostream>
#include <string>

#include "exportMacros.hpp" 
#include "taskUtils.hpp"
#include "mathUtils.hpp"
#include "demon.hpp"
#include "mouse.hpp"
#include "game.hpp"
#include "imgui.h"

class GAME_API RuntimeScript {
public:
    RuntimeScript(Demon& demon);
    virtual ~RuntimeScript();

    virtual void start();
    void start_update_task();
    void stop_update_task();

    int get_sort();
    int get_priority();
    virtual const std::string get_name();
    const std::unordered_map<std::string, bool>& get_buttons_map();

protected:
    Demon& demon;
    Game& game;
    ResourceManager& resource_manager;
    
    float dt;
    std::unordered_map<std::string, bool> input_map;

    // use this method with caution, because if you add an event listener
    // then you must manually remove it in destructor.
    template <typename Callable>
    void accept(const std::string& event_name, Callable callable) {
        demon.engine.accept(script_name, event_name, callable);
    }
    
    // use this method with caution, because if you add an event listener
    // then you must manually remove it in destructor.
    template <typename Callable>
    void add_event_listener(const std::string& uid, Callable callable) {
        demon.engine.add_event_listener(uid, callable);
    }

    void register_button_map(std::unordered_map<std::string, std::pair<std::string, bool>>& map);
    
    virtual void on_update(const PT(AsyncTask)&);
    virtual void on_event(const std::string& event_name);
    virtual void render_imgui();
    
    float get_dt();

private:
    std::string script_name;
    std::string task_name;
    PT(AsyncTask) update_task;
    std::unordered_map<std::string, std::pair<std::string, bool>> buttons_map_;
};

#define REGISTER_SCRIPT(ScriptClass)                                                 \
extern "C" GAME_API RuntimeScript* create_instance_##ScriptClass(Demon& demon) {     \
    return new ScriptClass(demon);                                                   \
}                                                                                    \
static ScriptRegistrar ScriptClass##_registrar(#ScriptClass);                        \

#endif // RUNTIME_SCRIPT_H
