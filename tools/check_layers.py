"""
Checks the engine's include rules: each module may only include the modules it is allowed to depend on,
and engine code never includes anything outside the engine (game or sample code).
Usage: python tools/check_layers.py   (from anywhere; exits 1 and lists violations if any rule is broken)
"""

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent

# Module -> engine modules it may include. "tests" may include anything in the engine.
ALLOWED = {
    "logging": {"logging"},
    "test_framework": {"test_framework"},
    "shared": {"shared", "logging"},
    "server": {"server", "shared", "logging"},
    "client": {"client", "shared", "logging"},
    "tools": {"shared", "logging"},
    "tests": None,
}

SOURCE_SUFFIXES = {".h", ".hpp", ".c", ".cpp", ".mm", ".m"}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"')


def module_of(path):
    """Module a file belongs to, from its path relative to the repo root; None if outside the scanned areas."""
    parts = path.relative_to(REPO_ROOT).parts
    if len(parts) >= 2 and parts[0] == "moksha":
        return parts[1]
    if parts[0] in ("tests", "tools"):
        return parts[0]
    return None


# Vendored files that live next to engine code instead of in a third_party folder.
VENDORED_FILES = {"d3dx12.h", "stb_perlin.h", "stb_perlin.cpp"}


def is_vendored(path):
    return "third_party" in path.relative_to(REPO_ROOT).parts or path.name in VENDORED_FILES


def check_file(path, module):
    problems = []
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    for number, line in enumerate(lines, start=1):
        match = INCLUDE_RE.match(line)
        if not match:
            continue
        target = match.group(1)
        where = f"{path.relative_to(REPO_ROOT).as_posix()}:{number}"

        if target.startswith("moksha/"):
            target_parts = target.split("/")
            target_module = target_parts[1] if len(target_parts) > 2 else None
            allowed = ALLOWED[module]
            if allowed is not None and target_module not in allowed:
                problems.append(f'{where}: {module} must not include "{target}"')
            elif not (REPO_ROOT / target).is_file():
                problems.append(f'{where}: "{target}" does not exist')
            continue

        # A non-qualified include is only fine for a file next to the includer (same folder).
        if (path.parent / target).is_file():
            continue
        problems.append(f'{where}: "{target}" is not an engine include (moksha/...) or a file in the same folder')
    return problems


def main():
    problems = []
    checked = 0
    for area in ("moksha", "tests", "tools"):
        root = REPO_ROOT / area
        if not root.is_dir():
            continue
        for path in sorted(root.rglob("*")):
            if path.suffix not in SOURCE_SUFFIXES or not path.is_file() or is_vendored(path):
                continue
            module = module_of(path)
            if module not in ALLOWED:
                problems.append(f"{path.relative_to(REPO_ROOT).as_posix()}: unknown module '{module}'")
                continue
            problems.extend(check_file(path, module))
            checked += 1

    for problem in problems:
        print(problem)
    print(f"check_layers: {checked} files checked, {len(problems)} problem(s).")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
