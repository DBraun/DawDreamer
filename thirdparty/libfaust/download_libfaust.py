import argparse
import os
import platform
import subprocess
from pathlib import Path


def download_file(url: str, output: str) -> None:
    if os.path.exists(output) and not args.force:
        print(f"File already exists: {output}")
    else:
        subprocess.run(["curl", "-L", url, "-o", output], check=True)

def install_windows(version: str) -> None:
    exe_file = f"Faust-{version}-win64.exe"
    download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{exe_file}", exe_file)
    cwd = str(Path(__file__).parent)
    subprocess.run([exe_file, "/S", f"/D={cwd}\\win64\\Release"], check=True)

def install_macos(version: str) -> None:
    for arch in ["arm64", "x64"]:
        dmg_file = f"Faust-{version}-{arch}.dmg"
        download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{dmg_file}", dmg_file)
        subprocess.run(["hdiutil", "attach", dmg_file], check=True)
        dir_path = f"darwin-{arch}/Release"
        os.makedirs(dir_path, exist_ok=True)
        subprocess.run(["cp", "-R", f"/Volumes/Faust-{version}/Faust-{version}/*", dir_path], check=True)
        subprocess.run(["hdiutil", "detach", f"/Volumes/Faust-{version}/"], check=True)

def install_linux(version: str) -> None:
    zip_file = f"libfaust-ubuntu-x86_64.zip"
    download_file(f"https://github.com/grame-cncm/faust/releases/download/{version}/{zip_file}", zip_file)
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
    parser = argparse.ArgumentParser(description="Download and install Libfaust.")
    parser.add_argument("-v", "--version", default="2.74.6", help="Specify the version of Faust to download.")
    parser.add_argument("--force", action="store_true", help="Force download even if files already exist.")
    args = parser.parse_args()

    main(args.version)
