import os
import subprocess
import sys
from pathlib import Path

def run_command(command, cwd=None):
    print(f"> {' '.join(command)}")
    result = subprocess.run(command, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"Error running command: {' '.join(command)}")
        sys.exit(result.returncode)

def build():
    # Paths
    script_dir = Path(__file__).resolve().parent
    wx_source = os.path.join(script_dir, "WxWidgets-3.3.0")
    wx_build = os.path.join(script_dir, "wx_build")
    wx_install = os.path.join(script_dir, "wx_install")

    # Create build directory if needed
    Path(wx_build).mkdir(parents=True, exist_ok=True)

    # Step 1: Configure wxWidgets
    cmake_configure_cmd = [
        "cmake",
        "-S", str(wx_source),
        "-B", str(wx_build),
        f"-DCMAKE_INSTALL_PREFIX={wx_install}",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DwxBUILD_SHARED=OFF",
        "-DwxBUILD_SAMPLES=OFF",
        "-DwxBUILD_DEMOS=OFF",
        "-DwxBUILD_TESTS=OFF",
        "-DwxBUILD_TOOLKIT=msw" if os.name == "nt" else "-DwxBUILD_TOOLKIT=gtk3"
    ]
    run_command(cmake_configure_cmd)

    # Step 2: Build and install
    cmake_build_cmd = [
        "cmake",
        "--build", str(wx_build),
        "--config", "Release",
        "--target", "install"
    ]
    run_command(cmake_build_cmd)

    print("\nwxWidgets build and install complete.")

# if __name__ == "__main__":
    # build()
