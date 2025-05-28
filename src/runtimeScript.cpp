#include "clockObject.h"
#include "asyncTaskManager.h"
#include "runtimeScript.hpp"

// Constructor
RuntimeScript::RuntimeScript(Demon& demon) :
    demon(demon),
    mouse(demon.engine.mouse),
    resource_manager(demon.engine.resource_manager),
    game(demon.game) {}

// Destructor
RuntimeScript::~RuntimeScript() {
    input_map.clear();
    buttons_map_.clear();
    demon.engine.remove_event_listener(script_name + "EventListener");
    demon.engine.ignore(script_name);
}

void RuntimeScript::start() {
    script_name = RuntimeScript::get_name();
    
    // Create update task
    update_task = make_task([this](AsyncTask* task) -> AsyncTask::DoneStatus {
        if (game.mouse_watcher->has_mouse() || 
            (this->demon.engine.mouse_watcher->has_mouse() && 
            this->demon.engine.mouse.is_mouse_centered())) {
            dt = ClockObject::get_global_clock()->get_dt();
            this->on_update(task);
        }
        return AsyncTask::DS_cont;
    }, "RuntimeScriptUpdate");

    // Add event listener for all kinds of events
    this->add_event_listener(
        script_name + "EventListener",
        [this](const std::string& event_name) { this->on_event(event_name); });

    // Accept relevant events
    demon.engine.accept(
        script_name,
        "game_mode_disabled",
        [this]() { this->stop_update_task(); });

    demon.engine.accept(script_name, "render_imgui", [this]() {
        ImGui::SetCurrentContext(demon.p3d_imgui.context_);
        this->render_imgui(); });
    
    // Start the update task and finalize
    start_update_task();
    std::cout << "SCRIPT: " << script_name << " started" << std::endl; 
}

void RuntimeScript::render_imgui() {}

// Getters
int RuntimeScript::get_sort() { return -1; }
int RuntimeScript::get_priority() { return -1; }

const std::unordered_map<std::string, bool>& RuntimeScript::get_buttons_map() {
    return input_map;
}

const std::string RuntimeScript::get_name() {
    std::string script_name = typeid(*this).name();
    size_t pos = script_name.find("class ");
    if (pos != std::string::npos) {
        script_name = script_name.substr(pos + 6);
    }
    return script_name;
}

// Register button map
void RuntimeScript::register_button_map(std::unordered_map<std::string, std::pair<std::string, bool>>& map) {
    input_map.clear();
    buttons_map_ = map;

    // Fill key map
    for (auto& it : buttons_map_) {
        if (input_map.find(it.second.first) == input_map.end()) {
            input_map[it.second.first] = false;
        }
    }
}

// Event handling
void RuntimeScript::on_update(const PT(AsyncTask)&) {}
 
void RuntimeScript::on_event(const std::string& event_name) {
    auto it = buttons_map_.find(event_name);
    if (it != buttons_map_.end()) {
        input_map[it->second.first] = it->second.second;
    }
}

// Get delta time
float RuntimeScript::get_dt() {
    return ClockObject::get_global_clock()->get_dt();
}

// Start update task
void RuntimeScript::start_update_task() {
    task_name = script_name + "Task";
    update_task->set_name(task_name);

    if (!has_task(task_name))
        AsyncTaskManager::get_global_ptr()->add(update_task);
}

// Stop update task
void RuntimeScript::stop_update_task() {
    remove_task(task_name);
}
