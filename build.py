# build.py
import os
import sys
import platform
import subprocess
import zipfile
import glob
import re
import shutil
import argparse
from pathlib import Path

# setup command line arguments
parser = argparse.ArgumentParser(
    description="Additional build options."
)
parser.add_argument("--wx", required=False, type=int, default=0, help="Build wx?")
args = parser.parse_args()

BUILD_WX = args.wx

# Detect OS and store in OS_TYPE
os_system = platform.system()
if os_system == "Darwin":
    OS_TYPE = "macOS"
elif os_system == "Linux":
    OS_TYPE = "Linux"
elif os_system.startswith(("CYGWIN", "MINGW", "MSYS")) or os.name == "nt":
    OS_TYPE = "Windows"
else:
    print(f"Unsupported OS: {os_system}")
    input("Press 'Enter' to continue...\n") if sys.stdout.isatty() else None
    sys.exit(1)

def pause_if_interactive():
    if sys.stdout.isatty():
        input("Press 'Enter' to continue...\n")

def check_command_success_status(*args):
    try:
        subprocess.run(args, check=True)
    except subprocess.CalledProcessError:
        print(f"Error: Command '{' '.join(args)}' failed.")
        pause_if_interactive()
        sys.exit(1)

def download(url, output):
    if shutil.which("curl"):
        check_command_success_status("curl", "--fail", "--silent", "--show-error", "--retry", "3",
                                     "--connect-timeout", "10", "--max-time", "360", "-#", "-L", url, "-o", output)
    elif shutil.which("powershell"):
        check_command_success_status("powershell", "-Command", f"Invoke-WebRequest -Uri '{url}' -OutFile '{output}'")
    else:
        print("Error: curl is not available and no fallback download method found.")
        pause_if_interactive()
        sys.exit(1)

def extract_zip(zip_file, destination):
    if shutil.which("unzip"):
        check_command_success_status("unzip", "-q", zip_file, "-d", destination)
    elif OS_TYPE == "Windows":
        if shutil.which("powershell"):
            check_command_success_status("powershell", "-Command", 
                f"Expand-Archive -Path '{zip_file}' -DestinationPath '{destination}' -Force")
        elif shutil.which("7z"):
            check_command_success_status("7z", "x", zip_file, f"-o{destination}", "-y")
        else:
            print("Error: No suitable extraction tool found (unzip, PowerShell, or 7-Zip).")
            pause_if_interactive()
            sys.exit(1)
    else:
        print("Error: unzip is required but not found.")
        pause_if_interactive()
        sys.exit(1)

# Globals
ROOT_DIR = os.getcwd()
DEPENDENCIES_DIR = os.path.join(ROOT_DIR, "extras")
THIRDPARTY_DIR = os.path.join(ROOT_DIR, "src", "thirdparty")
LOGs_DIR = os.path.join(ROOT_DIR, "logs")
BUILD_DIR = None

DEMO_PROJECTS  = {}
DEMO_PROJECTS_DIR = os.path.join(ROOT_DIR, "demos")
USERS_PROJECTS = {}
USER_PROJECTS_DIR = os.path.join(ROOT_DIR, "game")

# Build related vars
BUILD_TYPE = "Release"

# Executable directory, name and path
EXEC_DIR  = None
EXEC_NAME = None
EXEC_PATH = None

# Ensure necessary directories exist
# wx build is optional, so we create it in configure_wx function
for d in [DEPENDENCIES_DIR, THIRDPARTY_DIR, USER_PROJECTS_DIR, LOGs_DIR]:
    os.makedirs(d, exist_ok=True)

export_functions_file = ""  # DLL export functions file path
export_functions = []       # DLL export function names

def ensure_dependency(name, url, extract_to_dir, search_pattern=None):
    if search_pattern is None:
        search_pattern = f"{name.lower()}*"

    candidates = glob.glob(os.path.join(DEPENDENCIES_DIR, search_pattern))
    for candidate in candidates:
        if os.path.isfile(candidate) and candidate.endswith((".zip", ".7z", ".tar.gz")):
            print(f"Found compressed {name}: {os.path.basename(candidate)} — extracting...")
            extract_zip(candidate, extract_to_dir)
            return True

    # No archive found, download
    print(f"Downloading: {name}")
    archive_name = f"{name.lower()}.zip"
    final_path = os.path.join(DEPENDENCIES_DIR, archive_name)
    partial_path = final_path + ".partial"

    # Remove leftover from a failed download
    if os.path.exists(partial_path):
        print(f"Found incomplete download for {name}, deleting: {partial_path}")
        os.remove(partial_path)

    # Download to partial file
    download(url, partial_path)

    # Move to final filename after success
    os.rename(partial_path, final_path)

    print("Extracting...")
    extract_zip(final_path, extract_to_dir)
    print(f"{name} download completed successfully.\n")
    return True

def configure_cmake(config_file):
    print("Configuring Cmake")
    global cmake
    cmake = None
    
    cmake_dir = os.path.join(ROOT_DIR, "cmake")
    if not os.path.exists(cmake_dir):
        os.mkdir(cmake_dir)

    cmake_exe_dir = None # cmake executable directory
    
    def find_cmake():
        global cmake
        nonlocal cmake_exe_dir
        candidates = glob.glob(os.path.join(ROOT_DIR, "cmake", "cmake-*"))
        for d in candidates:
            cmake_exe = os.path.join(d, "bin", "cmake.exe")
            if os.path.isdir(d) and os.path.isfile(cmake_exe):
                cmake = cmake_exe
                cmake_exe_dir = os.path.join(d, "bin")
                return
        
    find_cmake()

    if not cmake:
        ensure_dependency(
            name="cmake",
            url="https://github.com/Kitware/CMake/releases/download/v3.30.7/cmake-3.30.7-windows-x86_64.zip",
            extract_to_dir=cmake_dir)   
        find_cmake()

    if not cmake:
        print("Error: Unable to find CMake.")
        pause_if_interactive()
        sys.exit(1)
        
    # Temporarily, add cmake path to system path
    os.environ["PATH"] = cmake_exe_dir + os.pathsep + os.environ["PATH"]
    
    # Configure CMake project if needed
    if not os.path.isfile(config_file):
        print("CMake configuration file not found.")
        panda3d_path = input("Enter the full path to Panda3D (e.g., C:/Panda3D-1.10.15-x64 or /opt/panda3d): ").replace("\\", "/")

        if not os.path.isdir(panda3d_path):
            print(f"Error: The provided Panda3D path '{panda3d_path}' does not exist. Exiting.")
            pause_if_interactive()
            sys.exit(1)

        print("Creating CMake configuration file...")
        with open(config_file, "w") as f:
            f.write(f'''# Path to the root installation of Panda3D
set(PANDA3D_ROOT "{panda3d_path}" CACHE STRING "Path to the Panda3D installation")

# Include and library directories
set(PANDA3D_INCLUDE_DIR "${{PANDA3D_ROOT}}/include")
set(PANDA3D_LIBRARY_DIR "${{PANDA3D_ROOT}}/lib")
''')

def configure_imgui():
    print("Configuring ImGUI")
    imgui_dir = os.path.join(THIRDPARTY_DIR, "imgui")

    if not os.path.isdir(imgui_dir):
        ensure_dependency(
            name="ImGui",
            url=f"https://github.com/ocornut/imgui/archive/refs/tags/v1.91.2.zip",
            extract_to_dir=DEPENDENCIES_DIR)

        found = next((d for d in glob.glob(os.path.join(DEPENDENCIES_DIR, "imgui-*")) if os.path.isdir(d)), None)
        if found:
            shutil.move(found, imgui_dir)
        else:
            print("Error: Unable to copy ImGUI to thirdparty/imgui folder.")
            pause_if_interactive()
            sys.exit(1)

def configure_wx():
    print("Configuring Wx")

    wx_dir = os.path.join(ROOT_DIR, "wx")
    wx_src_path = os.path.join(wx_dir, "wxWidgets-3.3.0")  # wx-source code path
    
    def test_wx(no_msg=False):
        test_executable = os.path.join(wx_dir, "test_app", "build", "Release", "wx-example.exe" if os.name == 'nt' else "wx-example")
        if os.path.isfile(test_executable):
            try:
                # print(f"Running test wx-executable: {test_executable}")
                subprocess.run([test_executable], check=True)
                # print("WxWidgets executable ran successfully.")
                return True
            except subprocess.CalledProcessError as e:
                if not no_msg:
                    print("wxWidgets test executable failed to run. Has wxWidgets built correctly?")
                return False
        else:
            if not no_msg:
                print("wxWidgets test executable failed to run. Has wxWidgets built correctly?")
            return False
    
    if not os.path.isdir(wx_src_path):
        os.makedirs(path, exist_ok=True)
        ensure_dependency(
            name="wxWidgets",
            url="https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.0/wxWidgets-3.3.0.zip",
            extract_to_dir=path)

    if not test_wx(no_msg = True):
        # Add the directory containing your local 'wx' to the front of sys.path
        sys.path.insert(0, WX_DIR)
        from buildWx import build as build_wx
        from buildTestWxApp import build as build_test_wx_app
        
        # Start build
        print("\nStarting to build wx....")
        build_wx()

        print("Starting to build test wx app....")
        build_test_wx_app()

    if not test_wx():
        pause_if_interactive()
        sys.exit(1)

def build_projects_info():
    base_dir = os.getcwd()

    annotations = {
        "game": "  # Main directory for user-defined projects",
        "demos": " # Directory for demo projects",
        "builds": "# Directory for build output"
    }
    
    demos_cout = 1
    game_cout = 1
    
    print("src")
    for top_level_dir in ["game", "demos", "builds"]:
        dir_path = os.path.join(base_dir, top_level_dir)
        if os.path.isdir(dir_path):
            parent_dir_name = os.path.basename(dir_path)
            print(f"├── {top_level_dir} {annotations.get(top_level_dir, '')}")
            subdirs = [d for d in glob.glob(os.path.join(dir_path, "*/")) if os.path.isdir(d)]
            for i, sub_dir in enumerate(subdirs, 1):
                dir_name = os.path.basename(os.path.normpath(sub_dir))
                if dir_name == "_assets":
                    continue
                if parent_dir_name == "demos":
                    DEMO_PROJECTS[demos_cout] = dir_name
                    demos_cout += 1
                if parent_dir_name == "game":
                    USERS_PROJECTS[game_cout] = dir_name
                    game_cout += 1
                prefix = "└──" if i == len(subdirs) else "├──"
                print(f"│   {prefix} {i}. {dir_name}")
            print("│")
    print("")

def get_user_project():
    global PROJECT_NAME, PROJECT_PATH
    break_outer_loop = False
    while True:
        if break_outer_loop:
            break
    
        project_name = input("Enter project name / index or -1 to exit: ").strip()

        if not project_name:
            print("Project name cannot be empty. Please try again.\n")
            continue
        elif project_name == "-1":
            sys.exit(0)
        elif project_name.startswith("-"):
            print("Invalid project name: Must not start with '-'.\n")
            continue
        elif re.search(r"[^a-zA-Z0-9_-]", project_name):
            print("Invalid project name: Only letters, numbers, hyphens, and underscores are allowed.\n")
            continue
        elif project_name in ["assets", "example_code"]:
            print("Invalid project name, try again.\n")
            continue
        elif re.match(r"^[GgDd](\d+)$", project_name):
            prefix = project_name[0]
            index = int(project_name[1:])

            if prefix.lower() == 'g' and index in USERS_PROJECTS:
                project_name = USERS_PROJECTS[index]
            elif prefix.lower() == 'd' and index in DEMO_PROJECTS:
                project_name = DEMO_PROJECTS[index]
            else:
                print(f"Invalid index: No project found at index {index}.\n")
                continue
        elif re.match(r"^\d+$", project_name):
            print("Invalid project name: Use 'G' or 'D' prefix for numeric selection.\n")
            continue

        game_path = Path.cwd() / "game" / project_name
        demo_path = Path.cwd() / "demos" / project_name

        if game_path.exists():
            PROJECT_PATH = game_path
            PROJECT_NAME = project_name
            break
        elif demo_path.exists():
            PROJECT_PATH = demo_path
            PROJECT_NAME = project_name
            break
        else:
            print(f"Project '{project_name}' does not exist.")
            while True:
                choice = input(f"Create project '{project_name}'? (y/n, -1 to exit): ").lower()
                if choice == 'y':
                    path = Path.cwd() / "game" / project_name
                    path.mkdir(parents=True, exist_ok=True)
                    PROJECT_PATH = path
                    PROJECT_NAME = project_name
                    break_outer_loop = True
                    break
                elif choice == 'n':
                    print("Please enter a different project name.\n")
                    break
                elif choice == "-1":
                    sys.exit(0)
                else:
                    print("Invalid input. Please enter 'y', 'n', or '-1'.\n")

def configure_project():
    if PROJECT_NAME and not (Path("demos") / PROJECT_NAME).exists() and \
        not (Path("game") / PROJECT_NAME).exists():
        print(f"Specified project {PROJECT_NAME} not found!")
        input("Press 'Enter' to continue...")
        sys.exit(1)

    if not any(PROJECT_PATH.iterdir()):
        with open(PROJECT_PATH / "MyScript.cpp", "w") as f:
            f.write("""#include \"runtimeScript.hpp\"

class MyScript : public RuntimeScript {
public:
    MyScript(Demon& demon) : RuntimeScript(demon) {

        /* your code goes here... */
    }
};
""")
    print(f"Starting project '{PROJECT_PATH.name}'\n")

def set_directories():
    global BUILD_DIR
    global EXEC_DIR
    global EXEC_NAME
    global EXEC_PATH
    
    if PROJECT_NAME:
        BUILD_DIR = Path.cwd() / "builds" / PROJECT_NAME
    else:
        BUILD_DIR = Path.cwd() / "builds" / "default-project"
        
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    EXEC_DIR  = BUILD_DIR / "Release"
    EXEC_NAME = "game.exe" if OS_TYPE == "Windows" else "game"
    EXEC_PATH = EXEC_DIR / EXEC_NAME

def generate_export_functions():
    global export_functions_file
    global export_functions

    # Set up paths for the export directory and output files
    export_dir = PROJECT_PATH / "exports"
    export_dir.mkdir(exist_ok=True)  # Ensure export directory exists
    export_functions_src = export_dir / "export_functions.cpp"  # Exported function src file
    
    export_functions_file = EXEC_DIR / "export_functions.txt"  # Exported function names .txt file

    # Recursively find all .cpp files in the project directory, excluding the export src file itself
    cpp_files = list(PROJECT_PATH.rglob("*.cpp"))
    cpp_files = [f for f in cpp_files if "export_functions" not in str(f)]

    # Export content containers
    includes = ['#include "runtimeScript.hpp"']  # Always include base class header
    export_bodies = []      # Export function definitions
    export_functions = []   # Exported functions names

    # Process .cpp files
    for cpp_file in cpp_files:
        class_name = cpp_file.stem  # Assume class name is the same as filename

        # Check if class inherits from RuntimeScript
        try:
            with cpp_file.open("r", encoding="utf-8") as f:
                content = f.read()
        except Exception as e:
            print(f"Could not read file {cpp_file}: {e}")
            continue

        if f"class {class_name} : public RuntimeScript" not in content:
            continue

        # Determine the include path: prefer .hpp file if it exists otherwise .cpp
        header_path = cpp_file.with_suffix(".hpp")
        rel_path = os.path.relpath(header_path if header_path.exists() else cpp_file, export_dir).replace("\\", "/")
        
        # Avoid duplicate includes
        include_stmt = f'#include "{rel_path}"'
        if include_stmt not in includes:
            includes.append(include_stmt)

        # Generate the export function definition for this class
        export_bodies.append(f'''extern "C" GAME_API RuntimeScript* create_instance_{class_name}(Demon& demon) {{
    return new {class_name}(demon);
}}''')

        # Add the function name to the list
        export_functions.append(f"create_instance_{class_name}")

    # Assemble and write the final C++ source file
    output = "// Auto-generated export functions\n" + "\n".join(includes) + "\n\n" + "\n".join(export_bodies)
    output += "\n"

    with export_functions_src.open("w", encoding="utf-8") as f:
        f.write(output)

    # Confirmation output
    print(f"Export functions generated: {export_functions_src}")

def log_cmake_output(log_file, *command):
    log_file = Path(log_file)
    if not log_file.parent.exists():
        log_file.parent.mkdir(parents=True)

    with open(log_file, "a", encoding="utf-8") as f:
        # Enable line-buffered text mode
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            bufsize=1,
            universal_newlines=True  # or text=True in Python 3.7+
        )

        for line in process.stdout:
            print(line, end="", flush=True)  # Immediate output to console
            f.write(line)
            f.flush()  # Make sure it's written to file immediately

        process.wait()
        if process.returncode != 0:
            print("Command failed, see log for details.")
            sys.exit(1)

def run_cmake_config():
    cmake_arch_option = []
    if OS_TYPE == "nt":
        cmake_arch_option = ["-A", "x64"]

    # Normalize project path to avoid backslash issues
    normalized_project_path = Path(PROJECT_PATH).as_posix()
    project_option = f"-DPROJECT_PATH={normalized_project_path}"

    build_log = LOGs_DIR + "/build-log.log"
    with open(build_log, 'w') as f:
        f.write("Starting Configuration with CMake...\n")
    log_cmake_output(build_log, cmake, f"-B{BUILD_DIR}", "-S.", *cmake_arch_option, project_option)

def run_cmake_build():
    build_log = LOGs_DIR + "/build-log.log"
    with open(build_log, 'a') as f:
        f.write("\n\nStarting Build...\n")

    log_cmake_output(build_log, cmake, "--build", str(BUILD_DIR), "--config", "Release")

def create_runtime_config():
    release_dir = BUILD_DIR / "Release"
    release_dir.mkdir(parents=True, exist_ok=True)

    config = {
        "working_dir": str(Path.cwd().resolve().as_posix()),
        "USER_PROJECTS_DIR": str(PROJECT_PATH.resolve().as_posix()),
        "shared_assets": str((Path.cwd() / "demos" / "_assets").resolve().as_posix()),
        "run_mode": "release"
    }

    config_path = release_dir / "game_config.txt"
    with open(config_path, "w") as f:
        for key, value in config.items():
            f.write(f"{key}: {value}\n")

    print(f"Runtime config generated at {config_path}")

def run_executable():
    print(f"Checking for executable in {EXEC_PATH}")

    if EXEC_PATH.exists():
        print("Starting the game executable...")
        subprocess.run([str(EXEC_PATH)])
        input("Press 'Enter' to continue...")
    else:
        print(f"Error: Executable '{EXEC_NAME}' not found in {BUILD_DIR}.")
        input("Press 'Enter' to continue...")
        sys.exit(1)

def finalize():
    # Write the plain-text list of exported function names generated 
    # in 'generate_export_functions'. We write them at the end, 
    # after build directories have been created.
    with export_functions_file.open("w", encoding="utf-8") as f:
        f.write("\n".join(export_functions))

# start
print("\nPandaEditor Project Configuration and Build System.\n")
print("Starting to configure dependencies...")

configure_cmake("config.cmake")
configure_imgui()
if BUILD_WX == 1:
    configure_wx()

print("Dependencies configured, Starting build...\n")

build_projects_info()
get_user_project()
configure_project()
set_directories()
generate_export_functions()
run_cmake_config()
run_cmake_build()
create_runtime_config()
finalize()

# run the executable
run_executable()
