#include "dllLoader.hpp"
#include "demon.hpp"
#include "runtimeScript.hpp"

bool DllLoader::load_script_dll(
    const std::string& function_name,
    const std::string& dll_path,
    Demon& demon) {
    return load_script_dll(std::vector<std::string>{ function_name }, dll_path, demon);
}

bool DllLoader::load_script_dll(
    const std::vector<std::string>& dll_functions,
    const std::string& dll_path,
    Demon& demon) {
    hDLL = LoadLibrary(dll_path.c_str());
    if (!hDLL) {
        std::cerr << "Failed to load DLL: " << dll_path
                  << " Error: " << GetLastError() << std::endl;
        return false;
    }

    // First pass: Load function pointers
    std::unordered_map<std::string, CreateInstanceFunc> function_map;
    bool all_functions_resolved = true;

    for (const auto& func_name : dll_functions) {
        std::cout << func_name << std::endl;
        CreateInstanceFunc create_instance =
            (CreateInstanceFunc)GetProcAddress(hDLL, func_name.c_str());

        if (!create_instance) {
            std::cerr << "Failed to get function '" << func_name
                      << "' in " << dll_path
                      << ". Error: " << GetLastError() << std::endl;
            all_functions_resolved = false;
            continue;
        }

        function_map[func_name] = create_instance;
    }

    if (!all_functions_resolved || function_map.empty()) {
        FreeLibrary(hDLL);
        return false;
    }
    
    // Second pass: Create instances and store them
    bool all_scripts_created = true;
    std::vector<std::string> created_script_names;

    for (const auto& pair : function_map) {
        const std::string& func_name = pair.first;
        CreateInstanceFunc create_instance = pair.second;

        RuntimeScript* script = create_instance(demon);
        if (!script) {
            std::cerr << "Failed to create script instance from "
                      << func_name << " in " << dll_path << std::endl;
            all_scripts_created = false;
            break;
        }

        std::string script_name = script->get_name();
        loaded_scripts[script_name] = { script };
        created_script_names.push_back(script_name);

        std::cout << "Successfully created script: "
                  << script_name << " from " << dll_path << std::endl;
    }

    // Clean up if any creation failed
    if (!all_scripts_created) {
        unload_all_scripts();
        return false;
    }

    // Third pass: Call start() on all successfully created scripts
    for (const std::string& script_name : created_script_names) {
        auto it = loaded_scripts.find(script_name);
        if (it != loaded_scripts.end()) {
            it->second.script_instance->start();
        }
    }

    for (auto& pair : loaded_scripts) {
        auto& script = pair.second;
        std::cout << "Script Found: " << script.script_instance->get_name() << std::endl;
    }

    return true;
}

void DllLoader::unload_all_scripts() {
    std::cout << "Unloading DLLs." << std::endl;
    for (auto& pair : loaded_scripts) {
        auto& script = pair.second;
        std::cout << "Unloaded: " << script.script_instance->get_name() << std::endl;
        delete script.script_instance;
        
    }
    loaded_scripts.clear();
    FreeLibrary(hDLL);
}

RuntimeScript* DllLoader::get_script(const std::string& name) const {
    auto it = loaded_scripts.find(name);
    if (it != loaded_scripts.end()) {
        std::cout << "Script: " << name << " found." << std::endl;
        return it->second.script_instance;
    }
    std::cout << "Script: " << name << " not found." << std::endl;
    return nullptr; // Not found
}

UserScriptsReg& UserScriptsReg::get_instance() {
    static UserScriptsReg instance;
    return instance;
}

void UserScriptsReg::register_script(const std::string& script) {
    std::string dll_name_prefix = "create_instance_";
    scripts.push_back( dll_name_prefix.append(script) );
}

const std::vector<std::string>& UserScriptsReg::get_scripts() const {
    return scripts;
}

ScriptRegistrar::ScriptRegistrar(const std::string& name) {
    UserScriptsReg::get_instance().register_script(name);
}
