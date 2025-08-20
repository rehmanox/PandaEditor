#ifndef DLL_LOADER_HPP
#define DLL_LOADER_HPP

#include <unordered_map>
#include <windows.h>
#include <iostream>
#include <string>

#include "exportMacros.hpp"

class RuntimeScript;
class Demon;

class DllLoader {
public:
    using CreateInstanceFunc = RuntimeScript* (*)(Demon&);
    HMODULE hDLL;
    DllLoader() = default;
    ~DllLoader() { /*unload_all_scripts();*/ }

    bool load_script_dll(
        const std::string& function_name,
        const std::string& dll_path,
        Demon& demon);

    bool load_script_dll(
        const std::vector<std::string>& dll_functions,
        const std::string& dll_path,
        Demon& demon);

    void unload_all_scripts();
    RuntimeScript* get_script(const std::string& name) const;

private:
    struct ScriptModule {
        RuntimeScript* script_instance;
    };

    std::unordered_map<std::string, ScriptModule> loaded_scripts; // key: script name
};

class UserScriptsReg {
public:
    static UserScriptsReg& get_instance();
    void register_script(const std::string& name);
    const std::vector<std::string>& get_scripts() const;
private:
    std::vector<std::string> scripts;
};

struct ENGINE_API ScriptRegistrar {
    ScriptRegistrar(const std::string& name);
};

#endif // DLL_LOADER_HPP
