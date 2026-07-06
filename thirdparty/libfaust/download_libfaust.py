import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path
from time import sleep


def download_file(url: str, output: str, force: bool = False) -> bool:
    """Download a file with curl.

    Args:
        url: The URL to download.
        output: The local file path to write to.
        force: If True, re-download even if the file already exists.

    Returns:
        True if a file was downloaded, False if it already existed.
    """
    if os.path.exists(output) and not force:
        print(f"File already exists: {output}")
        return False
    subprocess.run(["curl", "-L", url, "-o", output], check=True)
    return True


def install_windows(version: str, force: bool = False) -> None:
    """Download and install libfaust for Windows into win64/Release."""
    exe_file = f"Faust-{version}-win64.exe"
    url = f"https://github.com/grame-cncm/faust/releases/download/{version}/{exe_file}"
    if download_file(url, exe_file, force=force):
        cwd = str(Path(__file__).parent)
        subprocess.run([exe_file, "/S", f"/D={cwd}\\win64\\Release"], check=True)


def install_macos(version: str, force: bool = False) -> None:
    """Download and install libfaust for macOS into darwin-{arch}/Release."""
    for arch in ["arm64", "x64"]:
        dmg_file = f"Faust-{version}-{arch}.dmg"
        url = f"https://github.com/grame-cncm/faust/releases/download/{version}/{dmg_file}"
        if download_file(url, dmg_file, force=force):
            subprocess.run(["hdiutil", "attach", dmg_file], check=True)
            dir_path = f"darwin-{arch}/Release"
            shutil.copytree(f"/Volumes/Faust-{version}/Faust-{version}", dir_path, dirs_exist_ok=True)
            subprocess.run(["hdiutil", "detach", f"/Volumes/Faust-{version}/"], check=True)
            sleep(1)  # this seems to prevent an issue where the second DMG is copied to both destinations


def install_linux(version: str, arch: str, force: bool = False) -> None:
    """Download and install libfaust for Linux into ubuntu-{arch}/Release.

    Args:
        version: The Faust release version, such as "2.85.9".
        arch: The target architecture, either "x86_64" or "aarch64".
        force: If True, re-download even if the zip already exists.
    """
    asset = f"libfaust-ubuntu-{arch}.zip"
    # The release asset name is not versioned, so stamp the local file with the
    # version to avoid silently reusing a zip from an older release.
    zip_file = f"libfaust-{version}-ubuntu-{arch}.zip"
    url = f"https://github.com/grame-cncm/faust/releases/download/{version}/{asset}"
    if download_file(url, zip_file, force=force):
        dir_path = f"ubuntu-{arch}/Release"
        # Remove any previously extracted release so stale files don't linger.
        shutil.rmtree(dir_path, ignore_errors=True)
        os.makedirs(dir_path, exist_ok=True)
        subprocess.run(["unzip", "-o", zip_file, "-d", dir_path], check=True)


def main(version: str, arch: str, force: bool = False) -> None:
    """Install libfaust for the current operating system."""
    system = platform.system()
    if system == "Windows":
        install_windows(version, force=force)
    elif system == "Darwin":
        install_macos(version, force=force)
    elif system == "Linux":
        install_linux(version, arch, force=force)
    else:
        raise RuntimeError(f"Unknown operating system: {system}.")


if __name__ == "__main__":
    MIN_PYTHON = (3, 8)
    if sys.version_info < MIN_PYTHON:
        sys.exit("Python %s.%s or later is required.\n" % MIN_PYTHON)

    parser = argparse.ArgumentParser(description="Download and install Libfaust.")
    parser.add_argument("-v", "--version", default="2.85.9", help="Specify the version of Faust to download.")
    parser.add_argument(
        "--arch",
        default=platform.machine(),
        choices=["x86_64", "aarch64"],
        help="Target architecture (Linux only).",
    )
    parser.add_argument("--force", action="store_true", help="Force download even if files already exist.")
    args = parser.parse_args()

    main(args.version, args.arch, force=args.force)
