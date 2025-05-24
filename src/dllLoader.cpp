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
    HMODULE hDLL = LoadLibrary(dll_path.c_str());
    if (!hDLL) {
        std::cerr << "Failed to load DLL: " << dll_path 
        << " Error: " << GetLastError() << std::endl;
        return false;
    }
    
    bool all_scripts_loaded = true;
    
    for (const auto& func_name : dll_functions) {
        // Get the function pointer
        CreateInstanceFunc create_instance = 
            (CreateInstanceFunc)GetProcAddress(hDLL, func_name.c_str());

        if (!create_instance) {
            std::cerr << "Failed to get function '" << func_name 
            << "' in " << dll_path 
            << ". Error: " << GetLastError() << std::endl;
            all_scripts_loaded = false;
            continue; // Skip this one and try the rest
        }

        // Create script instance
        RuntimeScript* script = create_instance(demon);
        if (!script) {
            std::cerr << "Failed to create script instance from " 
            << func_name << " in " << dll_path << std::endl;
            all_scripts_loaded = false;
            continue;
        }

        std::cout << "Successfully loaded script: " 
        << script->get_name() << " from " << dll_path << std::endl;

        script->start();
        loaded_scripts.push_back({ hDLL, script });
    }
    
    if (!all_scripts_loaded)
        unload_all_scripts();
    
    // Success if at least one script was loaded
    return !dll_functions.empty();
}

void DllLoader::unload_all_scripts() {
    for (auto& script : loaded_scripts) {
        delete script.script_instance;
        FreeLibrary(script.hDLL);
    }
    loaded_scripts.clear();
}
