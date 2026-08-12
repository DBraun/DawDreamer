"""Remove non-PIC ncurses objects from an aarch64 libfaustwithllvm.a.

The libfaust-ubuntu-aarch64.zip release bundles a distro-built (non-PIC)
ncurses inside libfaustwithllvm.a. Objects compiled without -fPIC use
R_AARCH64_ADR_PREL_PG_HI21 relocations against external symbols (cur_term,
stderr, ...), which the linker rejects when building a shared object such as
dawdreamer.so. The rest of the archive is PIC: its only direct page-relative
external reference is __dso_handle, which always binds locally.

Deleting the offending members makes the archive PIC-clean. LLVM's terminfo
symbols (setupterm, tigetnum, ...) then resolve from the system libtinfo,
which the Linux Makefile already links via -ltinfo. The x86_64 release does
not bundle ncurses at all, so this script is a no-op there (the relocation
code it searches for is aarch64-specific).
"""

import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

# R_AARCH64_ADR_PREL_PG_HI21: a direct page-relative address computation,
# only legal in a shared object when the target symbol binds locally.
R_AARCH64_ADR_PREL_PG_HI21 = 0x113


def find_nonpic_members(archive: Path) -> list[str]:
    """Return archive members with non-PIC relocations against external symbols.

    Args:
        archive: Path to a static library (.a).

    Returns:
        Sorted member names having an R_AARCH64_ADR_PREL_PG_HI21 relocation
        against a symbol that is undefined in that member (excluding
        __dso_handle, which the linker always resolves locally).
    """
    nm_out = subprocess.run(
        ["nm", "-A", str(archive)], capture_output=True, text=True, check=True
    ).stdout
    undefined: dict[str, set] = defaultdict(set)
    for line in nm_out.splitlines():
        match = re.match(r"[^:]+:([^:]+):(?:[0-9a-f]+)? *U (.+)", line)
        if match:
            undefined[match.group(1)].add(match.group(2))

    readelf_out = subprocess.run(
        ["readelf", "-r", str(archive)], capture_output=True, text=True, check=True
    ).stdout
    bad_members = set()
    current = None
    for line in readelf_out.splitlines():
        file_match = re.match(r"File: .*\((.+)\)", line)
        if file_match:
            current = file_match.group(1)
            continue
        reloc_match = re.match(r"[0-9a-f]+\s+[0-9a-f]{4,8}([0-9a-f]{8})\s+\S+\s+[0-9a-f]+\s+(\S+)", line)
        if not reloc_match:
            continue
        reloc_type = int(reloc_match.group(1), 16)
        symbol = reloc_match.group(2)
        if (
            reloc_type == R_AARCH64_ADR_PREL_PG_HI21
            and symbol != "__dso_handle"
            and symbol in undefined.get(current, ())
        ):
            bad_members.add(current)

    return sorted(bad_members)


def strip_nonpic_members(archive: Path) -> int:
    """Delete non-PIC members from the archive in place and refresh its index.

    Args:
        archive: Path to the static library to modify.

    Returns:
        The number of members deleted.
    """
    members = find_nonpic_members(archive)
    if not members:
        print(f"{archive}: no non-PIC members found; nothing to do.")
        return 0

    non_ncurses = [m for m in members if m.endswith(".cpp.o")]
    if non_ncurses:
        raise RuntimeError(
            f"Refusing to delete C++ objects that appear to be Faust/LLVM code: {non_ncurses}. "
            "The archive layout has changed; review this script."
        )

    subprocess.run(["ar", "d", str(archive), *members], check=True)
    subprocess.run(["ranlib", str(archive)], check=True)
    print(f"{archive}: deleted {len(members)} non-PIC ncurses members.")
    return len(members)


if __name__ == "__main__":
    default = Path(__file__).parent / "ubuntu-aarch64" / "Release" / "lib" / "libfaustwithllvm.a"
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else default
    if not target.is_file():
        raise FileNotFoundError(f"Archive not found: {target}")
    strip_nonpic_members(target)
