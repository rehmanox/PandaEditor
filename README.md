<div id="toc">
  <ul style="list-style: none">
    <summary>
      <h2>  PandaEditor is a powerful, open-source 3D development framework built on top of the Panda3D engine, designed to accelerate your game development, simulation, or visualization projects. With built-in support for C++ scripting and intuitive scene editing tools, PandaEditor enables rapid iteration and real-time feedback—bringing your ideas to life faster. </h2>
    </summary>
  </ul>
</div>
<p align="center">
  <a href="https://www.patreon.com/codecreateplay"><img src="https://img.shields.io/badge/Patreon-F96854?style=for-the-badge&logo=patreon&logoColor=white" /></a>
  <a href="https://github.com/CodeCreatePlay/P3D-PandaEditor"><img src="https://img.shields.io/badge/Itch.io-FA5C5C?style=for-the-badge&logo=itchdotio&logoColor=white" /></a>
  <a href="https://github.com/CodeCreatePlay/P3D-PandaEditor"><img src="https://img.shields.io/badge/Reddit-FF4500?style=for-the-badge&logo=reddit&logoColor=white" /></a>
  <a href="https://github.com/CodeCreatePlay/P3D-PandaEditor"><img src="https://img.shields.io/badge/Discord-5865F2?style=for-the-badge&logo=discord&logoColor=white" /></a>
</p>

## TableOfContents
1. [Prerequisites and Configuration]()
2. [Getting started]()

<h2 align="center">Prerequisites and Configuration</h2>

### Prerequisites
- Panda3D SDK
- Python (comes bundled with Panda3D)
- CMake (Version 3.14 or higher)
- ImGUI
- C++ Compiler
   - Windows: [Microsoft Visual Studio (with MSVC)](https://visualstudio.microsoft.com/vs/) -OR- [MSVC Build Tools](https://visualstudio.microsoft.com/downloads/?q=build+tools)
   - Linux or macOS: GCC or Clang

**Note: All dependencies except for Panda3D and C++ compilers (which you can download from the links above) are downloaded at runtime by the build system if they are not found on the system path.**

🛠️ PandaEditor is currently a work in progress and depending on when you are visiting this page, some of the mentioned features may not be available!  
**Currently only Windows OS is supported.**

<h2 align="center">Getting Started</h2>

### Install
- Make sure you have Panda3D installed and a C++ compiler based on your OS.
- Download the repository as a ZIP file and extract it to a location of your choice on your system.
- Run `build.py` script, it will automatically download and extract all necessary dependencies.

### Creating a new project
PandaEditor uses a project-based workflow managed through its build system. When creating a new project, the system generates some boilerplate code and sets up a basic scene template to help you get started quickly.

Follow the steps below to create or load a project.

1. Run the `build.py` script.
2. When prompted, enter the name of the project or its index number. When specifying the project by index, prefix the number with `d` for demos or `g` for games—for example, use `d1` to load the first demo project, or `g1` to load the first game project.
3. If the specified project does not exist, you will have the option to create it or choose another.

The system will first then search for the project in two directories:
- `game` → Stores user-created projects.
- `demos` → Contains built-in demo projects.

Each subfolder inside `game` or `demos` represents a separate project, and source files from only the selected project are included in the build.

**Example Project Directory Structure.**
```
src/
├── game/                     # Main directory for user-defined projects
│   ├── UserProject           # Example user project
│   ├── AnotherProject        # Another example user project
│   └── YetAnotherProject     # Additional example user project
│
├── demos/                    # Directory for demo projects
│   └── roaming-ralph         # Demo project: Roaming Ralph
│
├── builds/                   # Directory for build output
│   ├── UserProject           # Build output for UserProject
│   ├── AnotherProject        # Build output for AnotherProject
│   ├── YetAnotherProject     # Build output for YetAnotherProject
│   └── roaming-ralph         # Build output for Roaming Ralph demo
```

### Programming in PandaEditor
Programming with PandaEditor requires a beginner-level understanding of C++. Users can write runtime scripts by inheriting from the `RuntimeScript` base class to update / change the state of the system and define object behaviors.  
The `RuntimeScript` class provides direct access to core Panda3D and PandaEditor systems. It also offers several utility methods to simplify and enhance your scripting experience.

> 💡 1. When a new project is created, a sample `RuntimeScript` is automatically generated — use it as a reference for structure and usage.  

By default, all scripts in project folder which are drieved from `RuntimeScript` base class are considered as runtime scripts or editor scripts, provided that the class name and source file names are same. For further examples and practical usage, refer to the included demo projects.

```
#include "runtimeScript.hpp"

// Example User Script
class MyScript : public RuntimeScript {
public:
    MyScript(Demon& demon) : RuntimeScript(demon) {
        // Constructor — used for initialization

        // Accessing base class variables
        float dt = this->dt;  // Time elapsed since last frame

        // --------------------- Event Handling ---------------------
        // Bind Panda3D events to custom event handlers
        // 'accept' is defined in base class
        this->accept("window-resize", []() { /*Handle resize event*/ });

        // --------------------- Input Handling ---------------------
        // Define a mapping between key events (e.g., 'w', 'a-up') and abstract game actions (e.g., "forward", "left").
        // Each mapping associates an event string with a pair: {action_name, is_pressed}.
        //     - is_pressed = true  → key-down event (e.g., "w" pressed)
        //     - is_pressed = false → key-up event (e.g., "w-up" released)

        std::unordered_map<std::string, std::pair<std::string, bool>> buttons_map;

        // Movement input mappings (key down)
        buttons_map["a"] = {"left",      true};
        buttons_map["d"] = {"right",     true};
        buttons_map["w"] = {"forward",   true};
        buttons_map["e"] = {"cam-left",  true};
        buttons_map["q"] = {"cam-right", true};

        // Movement input mappings (key up)
        buttons_map["a-up"] = {"left",      false};
        buttons_map["d-up"] = {"right",     false};
        buttons_map["w-up"] = {"forward",   false};
        buttons_map["e-up"] = {"cam-left",  false};
        buttons_map["q-up"] = {"cam-right", false};

        // Register this input mapping with the base class.
        // This populates the internal input_map used to track action states.
        this->register_button_map(buttons_map);

        // After registration, you can check `input_map["forward"]` or similar in `on_update()`
        // to determine whether a given action (key) is currently active (pressed).
    }

protected:
    void on_update(const PT(AsyncTask)&) override {
        // This method is called every frame.

        // Example: Respond to input actions via input_map (set automatically)
        if (input_map.at("forward")) {
            // 'w' key is currently held down
        }
    }

    void on_event(const std::string& event_name) override {
        // Recieves events sent by Panda3D.
        // Print event_name to see how and when they are generated.
        RuntimeScript::on_event(event_name);
    }

    void render_imgui() override {
        // Place ImGui rendering logic here.
        // This function is called during the render_imgui event.
        // Avoid placing ImGui code outside of this method or its callees.
    }
};
```

**Editor scripts:** There are special types of `RuntimeScripts` called `EditorScripts` that execute in **Developer Mode** only, they will not be shipped along with the final executable. You can use them to create development tools or for debugging purposes.  
To specify a script as `EditorScripts` prefix the class name with `Editor_` for example 'Editor_FoliageSys'.

### Editor and Game Modes
In PandaEditor, the **Editor** and **Game** modes are distinct. Any changes made in **Game mode** are **not persistent** — they will be discarded once you exit the mode.

- **Editor Mode:** Use this mode to build scenes, place objects, set up lighting, cameras, and configure game world elements.  
- **Game Mode:** This mode runs your game for preview / testing. RuntimeScript execution only happens here.

The Game Viewport and Editor Viewport are also separate:  
- **Editor Viewport:** is the primary view where you interact with the scene.  
- **Game Viewport:** appears by default in the bottom-left corner and shows the game as it runs.

Some features — such as object manipulation handles, selection tools, and gizmos — are exclusive to the `Editor Viewport`.

### HotKeys
**ViewPort Camera Navigation:**
* **Truck and Jib (Horizontal and Vertical movement):** `alt + middle mouse button drag`
* **Dolly (Zoom):** `alt + right mouse button drag`
* **Orbit:** `alt + left mouse button drag`

**Other:**
* **Reload Runtime Scripts:** `shift + r`
* **Toggle Game Mode:** `shift + g`
* **Resize Game Viewport:** `Shift + (i / d)`
* **Change Game Viewport Position:** `shift + (1 / 2 / 3 / 4 / 0`)
* **Exit PandaEditor:** `shift + e`

### Common Issues
- **Unsupported Compiler** 
    - Ensure you're using a supported compiler MSVC on Windows.
    - GCC or Clang otherwise.
- **Panda3D installiation not found**
	- Ensure that cmake.config exists (it should be automatically generated by the build system) and that PANDA3D_ROOT is correctly set to the root directory of your Panda3D installation. See the example below.
```
	# Path to the root installation of Panda3D
	set(PANDA3D_ROOT "C:/Panda3D-1.10.15-x64" CACHE STRING "Path to the Panda3D installation")
	
	# Include and library directories
	set(PANDA3D_INCLUDE_DIR "${PANDA3D_ROOT}/include")
	set(PANDA3D_LIBRARY_DIR "${PANDA3D_ROOT}/lib")
```