import argparse
import os
import platform
import subprocess
import sys
import shutil
from pathlib import Path
from time import sleep


def download_file(url: str, output: str) -> bool:
    """ Return True if a file was downloaded, False otherwise."""
    if os.path.exists(output) and not args.force:
        print(f"File already exists: {output}")
        return False
    else:
        subprocess.run(["curl", "-L", url, "-o", output], check=True)
        return True

def install_windows(version: str) -> None:
    exe_file = f"Faust-{version}-win64.exe"
    if download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{exe_file}", exe_file):
        cwd = str(Path(__file__).parent)
        subprocess.run([exe_file, "/S", f"/D={cwd}\\win64\\Release"], check=True)
    libfaustwithllvm = f"{Path(__file__).parent}/win64/Release/lib/libfaustwithllvm.lib"
    assert os.path.isfile(libfaustwithllvm), f"Missing file: {libfaustwithllvm}"

def install_macos(version: str) -> None:
    for arch in ["arm64", "x64"]:
        dmg_file = f"Faust-{version}-{arch}.dmg"
        if download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{dmg_file}", dmg_file):
            subprocess.run(["hdiutil", "attach", dmg_file], check=True)
            dir_path = f"darwin-{arch}/Release"
            shutil.copytree(f"/Volumes/Faust-{version}/Faust-{version}", dir_path, dirs_exist_ok=True)
            subprocess.run(["hdiutil", "detach", f"/Volumes/Faust-{version}/"], check=True)
            sleep(1)  # this seems to prevent an issue where the second DMG is copied to both destinations

def install_linux(version: str) -> None:
    zip_file = f"libfaust-ubuntu-x86_64.zip"
    if download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{zip_file}", zip_file):
        dir_path = "ubuntu-x86_64/Release"
        os.makedirs(dir_path, exist_ok=True)
        subprocess.run(["unzip", zip_file, "-d", dir_path], check=True)

def main(version: str) -> None:
    system = platform.system()
    if system == "Windows":
        install_windows(version)
    elif system == "Darwin":
        install_macos(version)
    elif system == "Linux":
        install_linux(version)
    else:
        raise RuntimeError(f"Unknown operating system: {system}.")


if __name__ == "__main__":
    MIN_PYTHON = (3, 6)
    if sys.version_info < MIN_PYTHON:
        sys.exit("Python %s.%s or later is required.\n" % MIN_PYTHON)

    parser = argparse.ArgumentParser(description="Download and install Libfaust.")
    parser.add_argument("-v", "--version", default="2.74.6", help="Specify the version of Faust to download.")
    parser.add_argument("--force", action="store_true", help="Force download even if files already exist.")
    args = parser.parse_args()

    main(args.version)
