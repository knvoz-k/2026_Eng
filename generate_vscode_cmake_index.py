#!/usr/bin/env python3
"""
Generate a VS Code/CMake indexing setup for STM32CubeMX HAL projects.

Run this script from the project root. It prefers Keil MDK `.uvprojx` data when
available, then falls back to scanning the usual CubeMX folder layout.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


HAL_DRIVER_RE = re.compile(r"STM32([A-Z0-9]+)xx_HAL_Driver", re.IGNORECASE)
DEVICE_RE = re.compile(r"(STM32[A-Z0-9]+xx|STM32[A-Z0-9]+x[A-Z])")
STARTUP_RE = re.compile(r"startup_(stm32[a-z0-9]+)\.s$", re.IGNORECASE)


@dataclass
class ProjectInfo:
    root: Path
    project_name: str
    defines: list[str]
    include_dirs: list[Path]
    sources: list[Path]
    cpu: str
    fpu: str | None
    float_abi: str | None
    hal_driver_dir: Path | None


def cmake_path(path: Path, root: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return path.resolve().as_posix()


def cmake_subdir_path(path: Path, root: Path) -> str:
    rel_or_abs = cmake_path(path, root)
    if re.match(r"^[A-Za-z]:/", rel_or_abs) or rel_or_abs.startswith("/"):
        return f'"{rel_or_abs}"'
    return f'"${{CMAKE_CURRENT_SOURCE_DIR}}/../../{rel_or_abs}"'


def dedupe(items: Iterable) -> list:
    seen = set()
    result = []
    for item in items:
        key = str(item).lower() if isinstance(item, Path) else str(item)
        if key not in seen:
            seen.add(key)
            result.append(item)
    return result


def read_text(path: Path) -> str:
    for encoding in ("utf-8-sig", "utf-8", "gbk"):
        try:
            return path.read_text(encoding=encoding)
        except UnicodeDecodeError:
            continue
    return path.read_text(errors="ignore")


def find_uvprojx(root: Path) -> Path | None:
    candidates = sorted(root.glob("**/*.uvprojx"), key=lambda p: (len(p.parts), p.as_posix().lower()))
    return candidates[0] if candidates else None


def find_ioc(root: Path) -> Path | None:
    candidates = sorted(root.glob("*.ioc"), key=lambda p: p.name.lower())
    return candidates[0] if candidates else None


def parse_uvprojx(root: Path, uvprojx: Path) -> tuple[str | None, list[str], list[Path], list[Path], str | None, bool]:
    text = read_text(uvprojx)
    xml_root = ET.fromstring(text)

    target_name = xml_root.findtext(".//TargetName")
    defines: list[str] = []
    include_dirs: list[Path] = []
    sources: list[Path] = []

    for define_text in xml_root.findall(".//Cads//Define"):
        if define_text.text:
            defines.extend(x.strip() for x in re.split(r"[,;]", define_text.text) if x.strip())

    for include_text in xml_root.findall(".//Cads//IncludePath"):
        if include_text.text:
            for item in re.split(r"[;]", include_text.text):
                item = item.strip()
                if item:
                    include_dirs.append((uvprojx.parent / item).resolve())

    for file_path in xml_root.findall(".//Groups//FilePath"):
        if not file_path.text:
            continue
        candidate = (uvprojx.parent / file_path.text.strip()).resolve()
        if candidate.suffix.lower() in {".c", ".cpp", ".cc", ".cxx", ".s", ".S".lower()}:
            sources.append(candidate)

    cpu_text = xml_root.findtext(".//Cpu") or ""
    cpu_match = re.search(r'CPUTYPE\("([^"]+)"\)', cpu_text)
    cpu = cpu_match.group(1) if cpu_match else None
    has_fpu = "FPU" in cpu_text.upper()

    return target_name, defines, include_dirs, sources, cpu, has_fpu


def parse_ioc(root: Path, ioc: Path | None) -> tuple[str | None, str | None]:
    if not ioc:
        return None, None

    project_name = ioc.stem
    device = None
    for line in read_text(ioc).splitlines():
        if line.startswith("ProjectManager.ProjectName="):
            value = line.split("=", 1)[1].strip()
            if value:
                project_name = Path(value).stem
        elif line.startswith("Mcu.Name=") or line.startswith("Mcu.CPN="):
            value = line.split("=", 1)[1].strip()
            if value:
                device = value

    return project_name, device


def infer_hal_driver(root: Path) -> Path | None:
    drivers = sorted((root / "Drivers").glob("STM32*xx_HAL_Driver")) if (root / "Drivers").exists() else []
    return drivers[0].resolve() if drivers else None


def infer_family_from_hal(hal_driver_dir: Path | None) -> str | None:
    if not hal_driver_dir:
        return None
    match = HAL_DRIVER_RE.search(hal_driver_dir.name)
    return f"STM32{match.group(1).upper()}xx" if match else None


def infer_device_define(defines: list[str], root: Path, ioc_device: str | None, family: str | None) -> str | None:
    for item in defines:
        if DEVICE_RE.fullmatch(item):
            return item

    if ioc_device:
        device_upper = ioc_device.upper()
        if family:
            family_prefix = family[:-2]
            if device_upper.startswith(family_prefix):
                return family
        match = re.match(r"(STM32[A-Z]\d+)", device_upper)
        if match:
            return match.group(1) + "xx"

    for startup in root.glob("**/startup_stm32*.s"):
        match = STARTUP_RE.search(startup.name)
        if match:
            token = match.group(1).upper()
            return token[:-2] + "xx" if token.endswith("XX") else token[:9] + "xx"

    return family


def infer_cpu(cpu_from_uvprojx: str | None, device_define: str | None, has_fpu: bool) -> tuple[str, str | None, str | None]:
    if cpu_from_uvprojx:
        cpu = cpu_from_uvprojx.lower()
    else:
        cpu = "cortex-m4"
        if device_define:
            family = device_define.upper()
            if family.startswith(("STM32F0", "STM32G0", "STM32L0")):
                cpu = "cortex-m0"
            elif family.startswith(("STM32F1", "STM32F2", "STM32L1")):
                cpu = "cortex-m3"
            elif family.startswith(("STM32F3", "STM32F4", "STM32G4", "STM32L4", "STM32WB", "STM32WL")):
                cpu = "cortex-m4"
                has_fpu = True
            elif family.startswith(("STM32F7", "STM32H7")):
                cpu = "cortex-m7"
                has_fpu = True
            elif family.startswith(("STM32U5", "STM32H5", "STM32L5", "STM32WBA")):
                cpu = "cortex-m33"
                has_fpu = True

    fpu = None
    float_abi = None
    if has_fpu and cpu in {"cortex-m4", "cortex-m7", "cortex-m33"}:
        fpu = "fpv4-sp-d16" if cpu != "cortex-m7" else "fpv5-d16"
        float_abi = "hard"

    return cpu, fpu, float_abi


def fallback_include_dirs(root: Path, hal_driver_dir: Path | None, family: str | None) -> list[Path]:
    include_dirs = [
        root / "Core" / "Inc",
    ]

    if hal_driver_dir:
        include_dirs.extend([
            hal_driver_dir / "Inc",
            hal_driver_dir / "Inc" / "Legacy",
        ])

    cmsis_device = root / "Drivers" / "CMSIS" / "Device" / "ST"
    if family and (cmsis_device / family / "Include").exists():
        include_dirs.append(cmsis_device / family / "Include")
    elif cmsis_device.exists():
        include_dirs.extend(path for path in cmsis_device.glob("STM32*xx/Include"))

    include_dirs.append(root / "Drivers" / "CMSIS" / "Include")

    rte_dir = root / "MDK-ARM" / "RTE"
    if rte_dir.exists():
        include_dirs.extend(path for path in rte_dir.rglob("*") if path.is_dir())

    return [path.resolve() for path in include_dirs if path.exists()]


def fallback_sources(root: Path, hal_driver_dir: Path | None) -> list[Path]:
    sources: list[Path] = []
    for folder in (root / "Core" / "Src",):
        if folder.exists():
            sources.extend(sorted(folder.glob("*.c")))

    if hal_driver_dir and (hal_driver_dir / "Src").exists():
        sources.extend(sorted((hal_driver_dir / "Src").glob("*.c")))

    startup_files = sorted(root.glob("**/startup_stm32*.s"), key=lambda p: ("Drivers" in p.parts, len(p.parts)))
    if startup_files:
        sources.append(startup_files[0])

    return [path.resolve() for path in sources]


def collect_project_info(root: Path) -> ProjectInfo:
    uvprojx = find_uvprojx(root)
    ioc = find_ioc(root)
    ioc_project_name, ioc_device = parse_ioc(root, ioc)

    project_name = ioc_project_name or root.name
    defines: list[str] = []
    include_dirs: list[Path] = []
    sources: list[Path] = []
    cpu_from_uvprojx = None
    has_fpu = False

    if uvprojx:
        uv_name, defines, include_dirs, sources, cpu_from_uvprojx, has_fpu = parse_uvprojx(root, uvprojx)
        project_name = uv_name or project_name

    hal_driver_dir = infer_hal_driver(root)
    family = infer_family_from_hal(hal_driver_dir)
    device_define = infer_device_define(defines, root, ioc_device, family)

    if device_define and device_define not in defines:
        defines.append(device_define)
    if "USE_HAL_DRIVER" not in defines:
        defines.insert(0, "USE_HAL_DRIVER")

    if not include_dirs:
        include_dirs = fallback_include_dirs(root, hal_driver_dir, family)
    else:
        include_dirs.extend(fallback_include_dirs(root, hal_driver_dir, family))

    if not sources:
        sources = fallback_sources(root, hal_driver_dir)

    cpu, fpu, float_abi = infer_cpu(cpu_from_uvprojx, device_define, has_fpu)

    return ProjectInfo(
        root=root,
        project_name=project_name,
        defines=dedupe(defines),
        include_dirs=dedupe(path for path in include_dirs if path.exists()),
        sources=dedupe(path for path in sources if path.exists()),
        cpu=cpu,
        fpu=fpu,
        float_abi=float_abi,
        hal_driver_dir=hal_driver_dir,
    )


def render_list(paths_or_values: Iterable[str], indent: str = "    ") -> str:
    values = list(paths_or_values)
    return "\n".join(f"{indent}{value}" for value in values)


def render_top_cmake(info: ProjectInfo) -> str:
    return f"""cmake_minimum_required(VERSION 3.22)

# Generated by generate_vscode_cmake_index.py.
# Indexing project for VS Code/CMake Tools. Keep Keil MDK as build/debug source of truth.

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME {info.project_name})
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${{CMAKE_PROJECT_NAME}})
message("Build type: " ${{CMAKE_BUILD_TYPE}})

enable_language(C ASM)

add_executable(${{CMAKE_PROJECT_NAME}})
add_subdirectory(cmake/stm32cubemx)

target_link_libraries(${{CMAKE_PROJECT_NAME}}
    stm32cubemx
)
"""


def render_stm32cubemx_cmake(info: ProjectInfo) -> str:
    source_lines = render_list(cmake_subdir_path(path, info.root) for path in info.sources)
    include_lines = render_list(cmake_subdir_path(path, info.root) for path in info.include_dirs)
    define_lines = render_list(info.defines + ["$<$<CONFIG:Debug>:DEBUG>"])

    return f"""cmake_minimum_required(VERSION 3.22)

enable_language(C ASM)

set(MX_Defines_Syms
{define_lines}
)

set(MX_Include_Dirs
{include_lines}
)

set(MX_Sources
{source_lines}
)

add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${{MX_Include_Dirs}})
target_compile_definitions(stm32cubemx INTERFACE ${{MX_Defines_Syms}})

target_sources(${{CMAKE_PROJECT_NAME}} PRIVATE ${{MX_Sources}})
target_link_libraries(${{CMAKE_PROJECT_NAME}} stm32cubemx)
set_target_properties(${{CMAKE_PROJECT_NAME}} PROPERTIES ADDITIONAL_CLEAN_FILES ${{CMAKE_PROJECT_NAME}}.map)
"""


def render_toolchain(info: ProjectInfo) -> str:
    flags = [f"-mcpu={info.cpu}"]
    if info.fpu:
        flags.append(f"-mfpu={info.fpu}")
    if info.float_abi:
        flags.append(f"-mfloat-abi={info.float_abi}")
    target_flags = " ".join(flags)

    compiler = shutil.which("arm-none-eabi-gcc")
    compiler_dir = Path(compiler).parent.as_posix() if compiler else ""
    path_hint = f'list(APPEND CMAKE_PROGRAM_PATH "{compiler_dir}")\n' if compiler_dir else ""

    return f"""set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

{path_hint}set(TOOLCHAIN_PREFIX                arm-none-eabi-)

set(CMAKE_C_COMPILER                ${{TOOLCHAIN_PREFIX}}gcc)
set(CMAKE_ASM_COMPILER              ${{CMAKE_C_COMPILER}})
set(CMAKE_CXX_COMPILER              ${{TOOLCHAIN_PREFIX}}g++)
set(CMAKE_LINKER                    ${{TOOLCHAIN_PREFIX}}g++)
set(CMAKE_OBJCOPY                   ${{TOOLCHAIN_PREFIX}}objcopy)
set(CMAKE_SIZE                      ${{TOOLCHAIN_PREFIX}}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TARGET_FLAGS "{target_flags}")

set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} ${{TARGET_FLAGS}}")
set(CMAKE_ASM_FLAGS "${{CMAKE_C_FLAGS}} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -Wall -fdata-sections -ffunction-sections -fstack-usage")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

set(CMAKE_CXX_FLAGS "${{CMAKE_C_FLAGS}} -fno-rtti -fno-exceptions -fno-threadsafe-statics")
set(CMAKE_EXE_LINKER_FLAGS "${{TARGET_FLAGS}}")
set(TOOLCHAIN_LINK_LIBRARIES "m")
"""


def render_presets() -> str:
    return """{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "toolchainFile": "${sourceDir}/cmake/gcc-arm-none-eabi.cmake",
      "cacheVariables": {}
    },
    {
      "name": "Debug",
      "inherits": "default",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "Release",
      "inherits": "default",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "Debug",
      "configurePreset": "Debug"
    },
    {
      "name": "Release",
      "configurePreset": "Release"
    }
  ]
}
"""


def render_vscode_settings() -> str:
    return """{
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "cmake.useCMakePresets": "always",
  "cmake.configureOnOpen": true
}
"""


def render_c_cpp_properties(info: ProjectInfo) -> str:
    include_paths = ["${workspaceFolder}/**"]
    for path in info.include_dirs:
        include_paths.append(f"${{workspaceFolder}}/{cmake_path(path, info.root)}")

    config = {
        "configurations": [
            {
                "name": "STM32",
                "includePath": include_paths,
                "defines": info.defines,
                "compilerPath": "arm-none-eabi-gcc",
                "cStandard": "c11",
                "cppStandard": "c++17",
                "intelliSenseMode": "windows-gcc-arm",
                "compileCommands": "${workspaceFolder}/build/Debug/compile_commands.json",
                "browse": {
                    "path": include_paths,
                    "limitSymbolsToIncludedHeaders": True,
                },
            }
        ],
        "version": 4,
    }

    return json.dumps(config, indent=2, ensure_ascii=False) + "\n"


def write_file(path: Path, content: str, dry_run: bool) -> None:
    if dry_run:
        print(f"[dry-run] write {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    print(f"wrote {path}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Generate VS Code/CMake indexing files for STM32 HAL projects.")
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="STM32 project root, default: current directory")
    parser.add_argument("--dry-run", action="store_true", help="show what would be generated without writing files")
    args = parser.parse_args(argv)

    root = args.root.resolve()
    info = collect_project_info(root)

    if not info.sources:
        print("error: no C/ASM sources found. Run this from a CubeMX project root.", file=sys.stderr)
        return 1

    write_file(root / "CMakeLists.txt", render_top_cmake(info), args.dry_run)
    write_file(root / "CMakePresets.json", render_presets(), args.dry_run)
    write_file(root / "cmake" / "gcc-arm-none-eabi.cmake", render_toolchain(info), args.dry_run)
    write_file(root / "cmake" / "stm32cubemx" / "CMakeLists.txt", render_stm32cubemx_cmake(info), args.dry_run)
    write_file(root / ".vscode" / "settings.json", render_vscode_settings(), args.dry_run)
    write_file(root / ".vscode" / "c_cpp_properties.json", render_c_cpp_properties(info), args.dry_run)

    print()
    print(f"project: {info.project_name}")
    print(f"cpu: {info.cpu}")
    print(f"defines: {', '.join(info.defines)}")
    print(f"include dirs: {len(info.include_dirs)}")
    print(f"sources: {len(info.sources)}")
    print()
    print("Next: run `cmake --preset Debug`, or in VS Code run `CMake: Configure`.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
