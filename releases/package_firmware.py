#!/usr/bin/env python3
"""Package ESP-IDF or Arduino build outputs into flashable firmware archives."""

from __future__ import annotations

import argparse
import json
import os
import re
import stat
import sys
import zipfile
from pathlib import Path
from typing import Any


DEFAULT_ARDUINO_OFFSETS = (
    ("bootloader", "0x0"),
    ("partitions", "0x8000"),
    ("boot_app0", "0xe000"),
    ("app", "0x10000"),
)


def sanitize(value: str) -> str:
    value = value.strip().replace("\\", "/")
    value = re.sub(r"[^A-Za-z0-9._-]+", "-", value)
    value = value.strip(".-")
    return value or "firmware"


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def esp_idf_flash_files(build_dir: Path) -> tuple[list[str], list[dict[str, str]], dict[str, Any]]:
    flasher_args_path = build_dir / "flasher_args.json"
    if not flasher_args_path.is_file():
        raise FileNotFoundError(f"Missing {flasher_args_path.as_posix()}")

    flasher_args = read_json(flasher_args_path)
    write_flash_args = [str(item) for item in flasher_args.get("write_flash_args", [])]
    flash_files = flasher_args.get("flash_files", {})
    if isinstance(flash_files, dict):
        iterator = flash_files.items()
    elif isinstance(flash_files, list):
        iterator = ((item.get("offset"), item.get("file")) for item in flash_files if isinstance(item, dict))
    else:
        iterator = ()

    entries: list[dict[str, str]] = []
    for offset, file_name in iterator:
        if not offset or not file_name:
            continue
        source = build_dir / str(file_name)
        if not source.is_file():
            raise FileNotFoundError(f"Missing flash binary {source.as_posix()}")
        entries.append(
            {
                "offset": str(offset),
                "source": source.as_posix(),
                "archive_path": (Path("bin") / Path(str(file_name)).as_posix()).as_posix(),
            }
        )

    if not entries:
        raise ValueError("flasher_args.json does not contain flash file entries")

    extra = {"flasher_args": "flasher_args.json"}
    return write_flash_args, entries, extra


def classify_arduino_binary(path: Path) -> str:
    name = path.name.lower()
    if "merged" in name:
        return "merged"
    if "bootloader" in name:
        return "bootloader"
    if "partition" in name:
        return "partitions"
    if "boot_app0" in name:
        return "boot_app0"
    return "app"


def arduino_flash_files(build_dir: Path) -> tuple[list[str], list[dict[str, str]], dict[str, Any]]:
    binaries = sorted(build_dir.rglob("*.bin"), key=lambda item: item.as_posix().lower())
    if not binaries:
        raise FileNotFoundError(f"No Arduino .bin files found under {build_dir.as_posix()}")

    merged = [path for path in binaries if classify_arduino_binary(path) == "merged"]
    if merged:
        source = merged[0]
        return [], [{"offset": "0x0", "source": source.as_posix(), "archive_path": f"bin/{source.name}"}], {}

    by_kind = {classify_arduino_binary(path): path for path in binaries}
    entries: list[dict[str, str]] = []
    for kind, offset in DEFAULT_ARDUINO_OFFSETS:
        source = by_kind.get(kind)
        if source:
            entries.append({"offset": offset, "source": source.as_posix(), "archive_path": f"bin/{source.name}"})

    if not entries:
        raise ValueError(f"No recognized Arduino flash binaries found under {build_dir.as_posix()}")

    return [], entries, {}


def render_flash_sh(target: str, write_flash_args: list[str], entries: list[dict[str, str]]) -> str:
    lines = [
        "#!/usr/bin/env sh",
        "set -eu",
        'PORT="${1:-${ESPPORT:-PORT}}"',
        'if [ "$PORT" = "PORT" ]; then',
        '  echo "Usage: ./flash.sh <serial-port>" >&2',
        "  exit 2",
        "fi",
        "esptool.py \\",
        f"  --chip {target} \\",
        '  -p "$PORT" \\',
        "  write_flash \\",
    ]
    for arg in write_flash_args:
        lines.append(f"  {arg} \\")
    for index, entry in enumerate(entries):
        suffix = " \\" if index < len(entries) - 1 else ""
        lines.append(f"  {entry['offset']} {entry['archive_path']}{suffix}")
    return "\n".join(lines) + "\n"


def render_flash_bat(target: str, write_flash_args: list[str], entries: list[dict[str, str]]) -> str:
    flash_parts = write_flash_args + [part for entry in entries for part in (entry["offset"], entry["archive_path"])]
    return "\r\n".join(
        [
            "@echo off",
            "setlocal",
            'if "%~1"=="" (',
            "  echo Usage: flash.bat COMx",
            "  exit /b 2",
            ")",
            f"esptool.py --chip {target} -p %~1 write_flash " + " ".join(flash_parts),
            "endlocal",
        ]
    ) + "\r\n"


def write_text(archive: zipfile.ZipFile, name: str, text: str, executable: bool = False) -> None:
    info = zipfile.ZipInfo(name)
    info.external_attr = ((stat.S_IFREG | (0o755 if executable else 0o644)) << 16)
    archive.writestr(info, text)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--framework", choices=("esp-idf", "arduino"), required=True)
    parser.add_argument("--project", required=True)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--framework-version", required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--git-sha", default="")
    parser.add_argument("--output-dir", default="releases/dist")
    args = parser.parse_args()

    build_dir = Path(args.build_dir)
    if args.framework == "esp-idf":
        write_flash_args, entries, extra = esp_idf_flash_files(build_dir)
    else:
        write_flash_args, entries, extra = arduino_flash_files(build_dir)

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    archive_name = sanitize(args.name)
    archive_path = output_dir / f"{archive_name}.zip"

    manifest = {
        "name": archive_name,
        "framework": args.framework,
        "framework_version": args.framework_version,
        "project": Path(args.project).as_posix(),
        "target": args.target,
        "git_sha": args.git_sha,
        "write_flash_args": write_flash_args,
        "flash_files": [{"offset": entry["offset"], "path": entry["archive_path"]} for entry in entries],
        **extra,
    }

    with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        write_text(archive, "manifest.json", json.dumps(manifest, indent=2) + "\n")
        write_text(archive, "flash.sh", render_flash_sh(args.target, write_flash_args, entries), executable=True)
        write_text(archive, "flash.bat", render_flash_bat(args.target, write_flash_args, entries))
        flasher_args = build_dir / "flasher_args.json"
        if flasher_args.is_file():
            archive.write(flasher_args, "flasher_args.json")
        for entry in entries:
            archive.write(entry["source"], entry["archive_path"])

    print(archive_path.as_posix())
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise