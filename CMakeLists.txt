cmake_minimum_required(VERSION 3.14)
project(PandaEditor LANGUAGES CXX)

# ---------------- Options ---------------- #
option(BUILD_WX "Enable wxWidgets GUI integration" OFF)

# ---------------- C++ Standard ---------------- #
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    message(STATUS "Using Microsoft Visual Studio compiler.")
else()
    message(WARNING "PandaEditor is designed for MSVC on Windows.")
endif()

# ---------------- Directory Setup ---------------- #
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(THIRDPARTY_DIR ${SRC_DIR}/thirdparty)
set(STOCK_SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/stock/scripts)
set(GAME_SCRIPTS_DIR ${PROJECT_PATH})

if(EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    include("${CMAKE_SOURCE_DIR}/config.cmake")
else()
    message(FATAL_ERROR "Missing config.cmake. Please create it.")
endif()

# ---------------- Panda3D Setup ---------------- #
find_library(PANDA_FRAMEWORK p3framework PATHS ${PANDA3D_LIBRARY_DIR})
find_library(PANDA_LIB panda PATHS ${PANDA3D_LIBRARY_DIR})
find_library(PANDAEXPRESS_LIB pandaexpress PATHS ${PANDA3D_LIBRARY_DIR})
find_library(DTOOL_LIB p3dtool PATHS ${PANDA3D_LIBRARY_DIR})
find_library(DTOOLCONFIG_LIB p3dtoolconfig PATHS ${PANDA3D_LIBRARY_DIR})

foreach(lib_var PANDA_FRAMEWORK PANDA_LIB PANDAEXPRESS_LIB DTOOL_LIB DTOOLCONFIG_LIB)
    if(${lib_var} STREQUAL "${lib_var}-NOTFOUND")
        message(FATAL_ERROR "Missing Panda3D library: ${lib_var}")
    endif()
endforeach()

# ---------------- ImGui Setup ---------------- #
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)
if(EXISTS ${IMGUI_DIR})
    message(STATUS "ImGui found, linking with the project.")
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
    )
else()
    message(FATAL_ERROR "ImGui not found. Ensure it's in '${IMGUI_DIR}'.")
endif()

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# ---------------- wxWidgets Setup ---------------- #
if(BUILD_WX)
    set(wxWidgets_DIR "${CMAKE_SOURCE_DIR}/wx/wx-install/lib/cmake/wxWidgets")
    find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
endif()

# ---------------- Engine Library ---------------- #
file(GLOB_RECURSE ENGINE_SOURCE_FILES ${SRC_DIR}/*.cpp)
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX "${THIRDPARTY_DIR}/.*")
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX ".*runtimeScript\\.cpp$")
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp$")

add_library(engine_lib SHARED ${ENGINE_SOURCE_FILES})
target_compile_definitions(engine_lib PRIVATE ENGINE_DLL_EXPORTS)

# Export includes properly: consumers of engine_lib automatically inherit
target_include_directories(engine_lib
    PUBLIC
        ${SRC_DIR}/include
        ${SRC_DIR}/utils/include
        ${SRC_DIR}/imgui
        ${PANDA3D_INCLUDE_DIR}
        ${IMGUI_DIR}
)

if(BUILD_WX)
    target_include_directories(engine_lib PRIVATE ${wxWidgets_INCLUDE_DIRS})
endif()

target_link_libraries(engine_lib
    PUBLIC
        ${PANDA_FRAMEWORK} 
        ${PANDA_LIB} 
        ${PANDAEXPRESS_LIB} 
        ${DTOOL_LIB} 
        ${DTOOLCONFIG_LIB}
        imgui
)

if(BUILD_WX)
    target_link_libraries(engine_lib PRIVATE ${wxWidgets_LIBRARIES})
endif()

# ---------------- Game Executable ---------------- #
set(MAIN_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(game ${MAIN_SCRIPT})

# Only need to link engine_lib (includes are inherited)
target_link_libraries(game
    PRIVATE
        engine_lib
)

# ---------------- Script DLL ---------------- #
file(GLOB_RECURSE GAME_SCRIPTS ${GAME_SCRIPTS_DIR}/*.cpp)
file(GLOB_RECURSE STOCK_SCRIPTS ${STOCK_SCRIPTS_DIR}/*.cpp)

list(APPEND GAME_SCRIPTS ${STOCK_SCRIPTS})
list(APPEND GAME_SCRIPTS ${SRC_DIR}/runtimeScript.cpp)
list(FILTER GAME_SCRIPTS EXCLUDE REGEX ".*main\\.cpp$")

message(STATUS "Script files included in game_script.dll:")
foreach(FILE ${GAME_SCRIPTS})
    message(STATUS " - ${FILE}")
endforeach()

if(GAME_SCRIPTS)
    add_library(game_script SHARED ${GAME_SCRIPTS})
    target_compile_definitions(game_script PRIVATE GAME_DLL_EXPORTS)

    # Inherit includes from engine_lib + add scripts
    target_include_directories(game_script
        PRIVATE
            ${STOCK_SCRIPTS_DIR}
            ${STOCK_SCRIPTS_DIR}/include
    )

    target_link_libraries(game_script
        PRIVATE
            engine_lib
    )
else()
    message(WARNING "No script files found in ${GAME_SCRIPTS_DIR}. No DLLs will be built.")
endif()

# ---------------- Config File ---------------- #
file(WRITE "${CMAKE_BINARY_DIR}/config.txt.in" "@PROJECT_PATH@")
configure_file("${CMAKE_BINARY_DIR}/config.txt.in" "${CMAKE_BINARY_DIR}/config.txt")

# ---------------- Post-Build Option ---------------- #
option(ENABLE_POST_BUILD "Run the game after building" OFF)
if(ENABLE_POST_BUILD AND TARGET game)
    add_custom_command(TARGET game POST_BUILD
        COMMAND $<TARGET_FILE:game>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running game..."
    )
endif()
