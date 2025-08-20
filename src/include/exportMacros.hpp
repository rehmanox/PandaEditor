#ifndef EXPORT_MACROS_HPP
#define EXPORT_MACROS_HPP

// Engine API export/import
#ifdef ENGINE_DLL_EXPORTS
    #define ENGINE_API __declspec(dllexport)
#else
    #define ENGINE_API __declspec(dllimport)
#endif

// Ensure this matches the CMake define!
#ifdef GAME_DLL_EXPORTS
    #define GAME_API __declspec(dllexport)
#else
    #define GAME_API __declspec(dllimport)
#endif

#endif // EXPORT_MACROS_HPP
