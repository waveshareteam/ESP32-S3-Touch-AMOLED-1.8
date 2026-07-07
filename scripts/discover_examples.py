#!/usr/bin/env python3
"""Discover first-party examples for GitHub Actions matrices."""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
import re
import subprocess
from pathlib import Path


ESP_IDF_ROOT = Path("examples/esp-idf")
ARDUINO_ROOTS = (Path("examples/arduino"), Path("examples/arduino-v2"))
GLOBAL_PATTERNS = (
    ".github/workflows/examples.yml",
    "scripts/discover_examples.py",
    "releases/**",
    "config/**",
)


def sanitize_name(path: Path) -> str:
    return re.sub(r"[^A-Za-z0-9._-]+", "-", path.as_posix()).strip(".-")


def run_git(args: list[str]) -> list[str]:
    result = subprocess.run(["git", *args], check=True, text=True, stdout=subprocess.PIPE)
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def is_esp_idf_project(path: Path) -> bool:
    return (path / "CMakeLists.txt").is_file() and (path / "main").is_dir()


def list_esp_idf_examples() -> list[dict[str, str]]:
    if not ESP_IDF_ROOT.is_dir():
        return []
    examples = []
    for path in sorted(ESP_IDF_ROOT.iterdir(), key=lambda item: item.as_posix().lower()):
        if path.is_dir() and is_esp_idf_project(path):
            examples.append({"name": sanitize_name(path.relative_to(ESP_IDF_ROOT)), "path": path.as_posix()})
    return examples


def sketch_project_dir(sketch: Path) -> Path:
    if sketch.parent.name == sketch.stem:
        return sketch.parent
    return sketch.parent


def list_arduino_examples() -> list[dict[str, str]]:
    examples: list[dict[str, str]] = []
    for root in ARDUINO_ROOTS:
        first_party_root = root / "examples"
        libraries = root / "libraries"
        if not first_party_root.is_dir():
            continue
        seen: set[Path] = set()
        for sketch in sorted(first_party_root.rglob("*.ino"), key=lambda item: item.as_posix().lower()):
            project = sketch_project_dir(sketch)
            if project in seen:
                continue
            seen.add(project)
            rel = project.relative_to(first_party_root)
            examples.append(
                {
                    "name": sanitize_name(Path(root.name) / rel),
                    "path": project.as_posix(),
                    "libraries": libraries.as_posix(),
                }
            )
    return examples


def all_examples(surface: str) -> list[dict[str, str]]:
    if surface == "esp-idf":
        return list_esp_idf_examples()
    if surface == "arduino":
        return list_arduino_examples()
    raise ValueError(f"Unsupported surface: {surface}")


def matches_selector(example: dict[str, str], selector: str) -> bool:
    selector = selector.strip().strip("/")
    if not selector or selector == "all":
        return True
    path = example["path"].strip("/")
    if selector == path or selector == example["name"] or selector == Path(path).name:
        return True
    return path.endswith("/" + selector)


def changed_paths(base_ref: str | None, head_ref: str) -> list[str]:
    if base_ref:
        return run_git(["diff", "--name-only", f"{base_ref}...{head_ref}"])
    return run_git(["diff-tree", "--no-commit-id", "--name-only", "-r", head_ref])


def affected_by_paths(example: dict[str, str], paths: list[str], surface: str) -> bool:
    example_path = example["path"].strip("/")
    for changed in paths:
        changed = changed.strip().strip("/")
        if any(fnmatch.fnmatch(changed, pattern) for pattern in GLOBAL_PATTERNS):
            return True
        if changed == example_path or changed.startswith(example_path + "/"):
            return True
        if surface == "arduino":
            libraries = example.get("libraries", "").strip("/")
            if libraries and (changed == libraries or changed.startswith(libraries + "/")):
                return True
    return False


def select_examples(surface: str, selector: str, base_ref: str | None, head_ref: str, fallback_all: bool) -> list[dict[str, str]]:
    examples = all_examples(surface)
    selector = selector.strip().strip("/")
    if selector:
        selected = [example for example in examples if matches_selector(example, selector)]
    else:
        paths = changed_paths(base_ref, head_ref)
        selected = [example for example in examples if affected_by_paths(example, paths, surface)]
        if fallback_all and not selected:
            selected = examples
    return selected


def build_matrix(args: argparse.Namespace, selected: list[dict[str, str]]) -> dict[str, list[dict[str, str]]]:
    include: list[dict[str, str]] = []
    if args.surface == "esp-idf":
        versions = [item.strip() for item in args.idf_versions.split(",") if item.strip()]
        for example in selected:
            for idf in versions:
                include.append({"name": example["name"], "path": example["path"], "idf": idf})
    else:
        for example in selected:
            include.append(
                {
                    "name": example["name"],
                    "path": example["path"],
                    "libraries": example["libraries"],
                    "core": args.arduino_core,
                    "fqbn": args.fqbn,
                }
            )
    return {"include": include}


def write_github_output(path: str, matrix: dict[str, list[dict[str, str]]]) -> None:
    if not path:
        return
    count = len(matrix["include"])
    with open(path, "a", encoding="utf-8") as output:
        output.write(f"matrix={json.dumps(matrix, separators=(',', ':'))}\n")
        output.write(f"count={count}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--surface", choices=("esp-idf", "arduino"), required=True)
    parser.add_argument("--selector", default="")
    parser.add_argument("--base-ref")
    parser.add_argument("--head-ref", default="HEAD")
    parser.add_argument("--fallback-all", action="store_true")
    parser.add_argument("--idf-versions", default="v5.5.4,v6.0.2")
    parser.add_argument("--arduino-core", default="3.3.10")
    parser.add_argument("--fqbn", default="esp32:esp32:esp32s3")
    parser.add_argument("--github-output", default=os.environ.get("GITHUB_OUTPUT", ""))
    args = parser.parse_args()

    selected = select_examples(args.surface, args.selector, args.base_ref, args.head_ref, args.fallback_all)
    matrix = build_matrix(args, selected)
    write_github_output(args.github_output, matrix)
    print(json.dumps(matrix, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())