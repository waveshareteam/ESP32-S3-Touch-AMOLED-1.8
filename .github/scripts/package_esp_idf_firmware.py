#!/usr/bin/env python3
"""Package ESP-IDF build outputs into a flashable CI artifact."""

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


def sanitize(value: str) -> str:
    value = value.strip().replace("\\", "/")
    value = re.sub(r"[^A-Za-z0-9._-]+", "-", value)
    value = value.strip(".-")
    return value or "firmware"


def load_flasher_args(build_dir: Path) -> dict[str, Any]:
    path = build_dir / "flasher_args.json"
    if not path.is_file():
        raise FileNotFoundError(f"Missing {path.as_posix()}")
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def collect_flash_files(flasher_args: dict[str, Any], build_dir: Path) -> list[dict[str, str]]:
    flash_files = flasher_args.get("flash_files", {})
    entries: list[dict[str, str]] = []

    if isinstance(flash_files, dict):
        iterator = flash_files.items()
    elif isinstance(flash_files, list):
        iterator = ((item.get("offset"), item.get("file")) for item in flash_files if isinstance(item, dict))
    else:
        iterator = ()

    seen: set[tuple[str, str]] = set()
    for offset, file_name in iterator:
        if not offset or not file_name:
            continue
        source = build_dir / str(file_name)
        if not source.is_file():
            raise FileNotFoundError(f"Missing flash binary {source.as_posix()}")
        archive_path = Path("bin") / Path(str(file_name)).as_posix()
        key = (str(offset), archive_path.as_posix())
        if key in seen:
            continue
        seen.add(key)
        entries.append(
            {
                "offset": str(offset),
                "source": source.as_posix(),
                "archive_path": archive_path.as_posix(),
            }
        )

    if not entries:
        raise ValueError("flasher_args.json does not contain flash_files entries")

    return entries


def render_flash_sh(target: str, write_flash_args: list[str], entries: list[dict[str, str]]) -> str:
    parts = [
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
        parts.append(f"  {arg} \\")
    for index, entry in enumerate(entries):
        suffix = " \\" if index < len(entries) - 1 else ""
        parts.append(f"  {entry['offset']} {entry['archive_path']}{suffix}")
    return "\n".join(parts) + "\n"


def render_flash_bat(target: str, write_flash_args: list[str], entries: list[dict[str, str]]) -> str:
    lines = [
        "@echo off",
        "setlocal",
        'if "%~1"=="" (',
        "  echo Usage: flash.bat COMx",
        "  exit /b 2",
        ")",
        f"esptool.py --chip {target} -p %~1 write_flash "
        + " ".join(write_flash_args + [item for entry in entries for item in (entry["offset"], entry["archive_path"])]),
        "endlocal",
    ]
    return "\r\n".join(lines) + "\r\n"


def write_text_to_zip(archive: zipfile.ZipFile, name: str, text: str, executable: bool = False) -> None:
    info = zipfile.ZipInfo(name)
    info.external_attr = ((stat.S_IFREG | (0o755 if executable else 0o644)) << 16)
    archive.writestr(info, text)


def github_output(name: str, value: str) -> None:
    output_path = os.environ.get("GITHUB_OUTPUT")
    if output_path:
        with open(output_path, "a", encoding="utf-8") as output:
            output.write(f"{name}={value}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", required=True)
    parser.add_argument("--idf-version", required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--build-dir")
    parser.add_argument("--output-dir", default="release-artifacts")
    args = parser.parse_args()

    project = Path(args.project)
    build_dir = Path(args.build_dir) if args.build_dir else project / "build"
    flasher_args = load_flasher_args(build_dir)
    entries = collect_flash_files(flasher_args, build_dir)
    write_flash_args = [str(item) for item in flasher_args.get("write_flash_args", [])]

    artifact_name = sanitize(f"firmware-{project.as_posix()}-{args.idf_version}-{args.target}")
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    archive_path = output_dir / f"{artifact_name}.zip"

    manifest = {
        "project": project.as_posix(),
        "idf_version": args.idf_version,
        "target": args.target,
        "flasher_args": "flasher_args.json",
        "write_flash_args": write_flash_args,
        "flash_files": [
            {"offset": entry["offset"], "path": entry["archive_path"]}
            for entry in entries
        ],
    }

    with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.write(build_dir / "flasher_args.json", "flasher_args.json")
        write_text_to_zip(archive, "manifest.json", json.dumps(manifest, indent=2) + "\n")
        write_text_to_zip(archive, "flash.sh", render_flash_sh(args.target, write_flash_args, entries), executable=True)
        write_text_to_zip(archive, "flash.bat", render_flash_bat(args.target, write_flash_args, entries))
        for entry in entries:
            archive.write(entry["source"], entry["archive_path"])

    github_output("archive", archive_path.as_posix())
    github_output("artifact_name", artifact_name)
    print(archive_path.as_posix())
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise
