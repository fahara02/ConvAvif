import json
import os
import platform
import re
import shlex
import shutil


def find_compilers():
    compilers = []
    is_windows = platform.system() == 'Windows'
    path_dirs = os.environ.get('PATH', '').split(os.pathsep)

    # Check additional toolchain-related environment variables
    additional_env_vars = ['CC', 'CXX', 'CPP', 'COMPILER_PATH', 'ARM_TOOLCHAIN', 'TOOLCHAIN_PATH']
    for var in additional_env_vars:
        value = os.environ.get(var)
        if value and os.path.isdir(value):
            path_dirs.append(value)

    # Regex patterns with ARM compilers prioritized
    c_flags = re.IGNORECASE if is_windows else 0
    c_pattern = re.compile(
        r'^(?:.*-)?'  # Allow prefixes
        r'(arm-none-eabi-gcc|armcc|armclang|gcc|clang|cc|icc|icx|tcc|pgcc|xlc|ecc|hcc|cl|sdcc|dcc|rsic|mingw)'  # ARM first!
        r'(?:-[\d.]+)?$',  # Version suffix
        c_flags
    )
    cxx_pattern = re.compile(
        r'^(?:.*-)?'
        r'(arm-none-eabi-g\+\+|armc\+\+|armclang\+\+|g\+\+|clang\+\+|c\+\+|icpc|icpx|pgc\+\+|xlC|h\+\+|cl|mingw)'
        r'(?:-[\d.]+)?$',
        c_flags
    )

    def is_executable(path):
        return os.path.isfile(path) and (is_windows or os.access(path, os.X_OK))

    # Scan all directories
    for dir_path in path_dirs:
        dir_path = os.path.expandvars(os.path.expanduser(dir_path.strip('"')))
        if not os.path.isdir(dir_path):
            continue
        
        try:
            entries = os.listdir(dir_path)
        except (PermissionError, OSError):
            continue

        for entry in entries:
            full_path = os.path.join(dir_path, entry)
            if not is_executable(full_path):
                continue

            base_name = os.path.splitext(entry)[0]
            if is_windows:
                base_name = base_name.lower()

            # Match C/C++ compilers
            languages = []
            if c_pattern.search(base_name):
                languages.append('C')
            if cxx_pattern.search(base_name):
                languages.append('C++')

            if languages:
                compilers.append({
                    "path": os.path.realpath(full_path),
                    "languages": languages
                })

    # Deduplicate
    seen = set()
    return [c for c in compilers if not (c["path"] in seen or seen.add(c["path"]))]

if __name__ == '__main__':
    print(json.dumps(find_compilers(), indent=2))