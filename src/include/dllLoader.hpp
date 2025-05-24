#ifndef DLL_LOADER_HPP
#define DLL_LOADER_HPP

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

class RuntimeScript;
class Demon;

class DllLoader {
public:
    using CreateInstanceFunc = RuntimeScript* (*)(Demon&);

    DllLoader() = default;
    ~DllLoader() { unload_all_scripts(); }

    bool load_script_dll(
        const std::string& function_name,
        const std::string& dll_path,
        Demon& demon);

    bool load_script_dll(
        const std::vector<std::string>& dll_functions,
        const std::string& dll_path,
        Demon& demon);

    void unload_all_scripts();

private:
    struct ScriptModule {
        HMODULE hDLL;
        RuntimeScript* script_instance;
    };

    std::vector<ScriptModule> loaded_scripts;
};

#endif // DLL_LOADER_HPP
