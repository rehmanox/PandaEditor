cmake_minimum_required(VERSION 3.14)
project(PandaEditor LANGUAGES CXX)

# ---------------- C++ Standard Setup ---------------- #
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure correct compiler
if(MSVC)
    message(STATUS "Using Microsoft Visual Studio compiler.")
else()
    message(WARNING "PandaEditor is designed for MSVC on Windows.")
endif()

# ---------------- DIRECTORY SETUP ---------------- #
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(THIRDPARTY_DIR ${SOURCE_DIR}/thirdparty)
set(SCRIPTS_DIR ${PROJECT_PATH})

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")
else()
    message(FATAL_ERROR "Missing config.cmake. Please create it.")
endif()

# ---------------- PANDA3D SETUP ---------------- #
find_library(PANDA_FRAMEWORK libp3framework PATHS ${PANDA3D_LIBRARY_DIR})
find_library(PANDA_LIB libpanda PATHS ${PANDA3D_LIBRARY_DIR})
find_library(PANDAEXPRESS_LIB libpandaexpress PATHS ${PANDA3D_LIBRARY_DIR})
find_library(DTOOL_LIB libp3dtool PATHS ${PANDA3D_LIBRARY_DIR})
find_library(DTOOLCONFIG_LIB libp3dtoolconfig PATHS ${PANDA3D_LIBRARY_DIR})

if(NOT PANDA_FRAMEWORK OR NOT PANDA_LIB OR NOT PANDAEXPRESS_LIB OR NOT DTOOL_LIB OR NOT DTOOLCONFIG_LIB)
    message(FATAL_ERROR "Missing Panda3D libraries. Check your installation.")
endif()

# ---------------- IMGUI SETUP ---------------- #
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)

if(EXISTS ${IMGUI_DIR})
    message(STATUS "ImGui found, linking with the project.")
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )
else()
    message(FATAL_ERROR "ImGui not found. Ensure it's in '${IMGUI_DIR}'.")
endif()

# ---------------- SHARED ENGINE LIBRARY ---------------- #
file(GLOB_RECURSE ENGINE_SOURCE_FILES ${SOURCE_DIR}/*.cpp)
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX "${THIRDPARTY_DIR}/.*")
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX ".*runtimeScript.cpp$")
list(FILTER ENGINE_SOURCE_FILES EXCLUDE REGEX ${SOURCE_DIR}/main.cpp)

add_library(engine_lib STATIC ${ENGINE_SOURCE_FILES} ${IMGUI_SOURCES})

target_include_directories(engine_lib PRIVATE 
    ${SOURCE_DIR}/include
    ${SOURCE_DIR}/utils/include
    ${SOURCE_DIR}/imgui
    ${PANDA3D_INCLUDE_DIR}
    ${IMGUI_DIR}
)

# Link against Panda3D libraries
target_link_libraries(engine_lib PRIVATE 
    ${PANDA_FRAMEWORK} 
    ${PANDA_LIB} 
    ${PANDAEXPRESS_LIB} 
    ${DTOOL_LIB} 
    ${DTOOLCONFIG_LIB}
)

# ---------------- GAME EXECUTABLE ---------------- #
set(MAIN_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(game ${MAIN_SCRIPT})

target_include_directories(game PRIVATE 
    ${SOURCE_DIR}/include
    ${SOURCE_DIR}/utils/include
    ${SOURCE_DIR}/imgui
    ${PANDA3D_INCLUDE_DIR}
    ${IMGUI_DIR}
)

target_link_libraries(game PRIVATE engine_lib)

# ---------------- SCRIPT DLL COMPILATION ---------------- #
file(GLOB_RECURSE SCRIPT_FILES ${SCRIPTS_DIR}/*.cpp)
list(APPEND SCRIPT_FILES ${SOURCE_DIR}/runtimeScript.cpp)
list(FILTER SCRIPT_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# Print all script files
message(STATUS "Script files included in game_script.dll:")
foreach(FILE ${SCRIPT_FILES})
    message(STATUS " - ${FILE}")
endforeach()

if(SCRIPT_FILES)
    add_library(game_script SHARED ${SCRIPT_FILES})

    # Define DLL export macro
    target_compile_definitions(game_script PRIVATE GAME_DLL_EXPORTS)

    target_include_directories(game_script PRIVATE 
        ${SOURCE_DIR}/include
        ${SOURCE_DIR}/utils/include
        ${SOURCE_DIR}/imgui
        ${PANDA3D_INCLUDE_DIR}
        ${IMGUI_DIR}  
    )
        
    # Link against Panda3D libraries
    target_link_libraries(game_script PRIVATE 
        ${PANDA_FRAMEWORK} 
        ${PANDA_LIB} 
        ${PANDAEXPRESS_LIB} 
        ${DTOOL_LIB} 
        ${DTOOLCONFIG_LIB}
    )
        
    # Link against shared engine library
    target_link_libraries(game_script PRIVATE engine_lib)
else()
    message(WARNING "No script files found in ${SCRIPTS_DIR}. No DLLs will be built.")
endif()

# ---------------- Write Config file ---------------- #
# Create config.txt.in template
file(WRITE "${CMAKE_BINARY_DIR}/config.txt.in" "@PROJECT_PATH@")

# Expand variable to actual value in config.txt
configure_file("${CMAKE_BINARY_DIR}/config.txt.in" "${CMAKE_BINARY_DIR}/config.txt")



# ---------------- OPTIONAL POST-BUILD ---------------- #
option(ENABLE_POST_BUILD "Run the game after building" OFF)
if(ENABLE_POST_BUILD)
    if(WIN32)
        add_custom_command(TARGET game POST_BUILD
            COMMAND $<TARGET_FILE:game>
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running game.exe..."
        )
    elseif(UNIX)
        add_custom_command(TARGET game POST_BUILD
            COMMAND $<TARGET_FILE:game>
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running game..."
        )
    endif()
endif()
