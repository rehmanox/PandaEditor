import os
import subprocess
import sys
from pathlib import Path

def run_command(command, cwd=None):
    result = subprocess.run(command, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"Error: Command failed: {' '.join(command)}")
        sys.exit(result.returncode)

def build():
    # Get the directory where this script is located
    script_dir = Path(__file__).resolve().parent
    source_dir = os.path.join(script_dir, "test_app")
    build_dir = os.path.join(source_dir, "build")

    # Make sure build directory exists
    Path(build_dir).mkdir(parents=True, exist_ok=True)

    # Step 1: Run cmake configure
    cmake_configure_cmd = [
        "cmake",
        "-S", str(source_dir),
        "-B", str(build_dir),
        "-DCMAKE_BUILD_TYPE=Release"
    ]
    run_command(cmake_configure_cmd)

    # Step 2: Run cmake build
    cmake_build_cmd = [
        "cmake",
        "--build", str(build_dir),
        "--config", "Release"  # works for both single- and multi-config generators
    ]
    run_command(cmake_build_cmd)

    print("\nBuild complete.")

# if __name__ == "__main__":
    # main()
