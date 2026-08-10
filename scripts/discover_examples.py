#!/usr/bin/env python3
"""Discover and fail-closed route first-party examples for GitHub Actions."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path


ESP_IDF_ROOT = Path("examples/esp-idf")
ARDUINO_ROOTS = (Path("examples/arduino"), Path("examples/arduino-v2"))
DOC_SUFFIXES = {".md", ".markdown", ".rst"}
ARCHIVE_SUFFIXES = {".bin", ".zip", ".7z", ".tar", ".gz", ".xz"}
GLOBAL_PREFIXES = (".github/workflows/", "tests/", "scripts/", "releases/", "repository_policy.json")


class DiffUnavailable(RuntimeError):
    """The required complete changed-file input was unavailable."""


def sanitize_name(path: Path) -> str:
    return re.sub(r"[^A-Za-z0-9._-]+", "-", path.as_posix()).strip(".-")


def is_esp_idf_project(path: Path) -> bool:
    return (path / "CMakeLists.txt").is_file() and (path / "main").is_dir()


def list_esp_idf_examples() -> list[dict[str, str]]:
    return [
        {"name": sanitize_name(path.relative_to(ESP_IDF_ROOT)), "path": path.as_posix()}
        for path in sorted(ESP_IDF_ROOT.iterdir(), key=lambda item: item.as_posix().lower())
        if path.is_dir() and is_esp_idf_project(path)
    ] if ESP_IDF_ROOT.is_dir() else []


def list_arduino_examples() -> list[dict[str, str]]:
    examples: list[dict[str, str]] = []
    for root in ARDUINO_ROOTS:
        first_party_root, libraries = root / "examples", root / "libraries"
        if not first_party_root.is_dir():
            continue
        seen: set[Path] = set()
        for sketch in sorted(first_party_root.rglob("*.ino"), key=lambda item: item.as_posix().lower()):
            project = sketch.parent
            if project in seen:
                continue
            seen.add(project)
            examples.append({"name": sanitize_name(Path(root.name) / project.relative_to(first_party_root)),
                             "path": project.as_posix(), "libraries": libraries.as_posix()})
    return examples


def all_examples(surface: str) -> list[dict[str, str]]:
    return list_esp_idf_examples() if surface == "esp-idf" else list_arduino_examples()


def matches_selector(example: dict[str, str], selector: str) -> bool:
    selector = selector.replace("\\", "/").strip().strip("/")
    path = example["path"].strip("/")
    return (not selector or selector == "all" or selector == path or selector == example["name"]
            or selector == Path(path).name or path.startswith(selector + "/")
            or ("/" not in selector and selector in Path(path).parts) or path.endswith("/" + selector))


def parse_changed_file_input(path: Path) -> list[str]:
    if not path.is_file():
        raise DiffUnavailable(f"changed-file input is unavailable: {path}")
    paths: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line:
            continue
        fields = line.split("\t")
        if len(fields) > 1 and re.fullmatch(r"[A-Z][0-9]*", fields[0]):
            paths.extend(fields[1:])  # name-status rename/copy records include old and new paths.
        else:
            paths.append(line)
    if not paths:
        raise DiffUnavailable("changed-file input is empty")
    return [path.replace("\\", "/").strip("/") for path in paths if path.strip()]


def git_changed_paths(base_ref: str | None, head_ref: str) -> list[str]:
    if not base_ref or set(base_ref) == {"0"}:
        raise DiffUnavailable("a complete base/head diff is required")
    try:
        result = subprocess.run(["git", "diff", "--name-status", "-z", f"{base_ref}...{head_ref}"],
                                check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except (OSError, subprocess.CalledProcessError) as exc:
        raise DiffUnavailable(f"unable to read changed files: {exc}") from exc
    fields = result.stdout.decode("utf-8", "surrogateescape").split("\0")
    paths: list[str] = []
    index = 0
    while index < len(fields) - 1:
        status = fields[index]
        index += 1
        if not status:
            continue
        if status[0] in "RC":
            if index + 1 >= len(fields):
                raise DiffUnavailable("incomplete rename/copy diff record")
            paths.extend((fields[index], fields[index + 1]))
            index += 2
        else:
            if index >= len(fields):
                raise DiffUnavailable("incomplete diff record")
            paths.append(fields[index])
            index += 1
    if not paths:
        raise DiffUnavailable("changed-file diff is empty")
    return [path.replace("\\", "/").strip("/") for path in paths if path.strip()]


def is_documentation(path: str) -> bool:
    lower = path.lower()
    return (Path(lower).suffix in DOC_SUFFIXES or lower.startswith("docs/")
            or lower in {"code_of_conduct.md", "contributing.md", "security.md", "support.md", "third_party.md"}
            or lower.startswith(".github/issue_template/") or lower == ".github/pull_request_template.md")


def classify_paths(paths: list[str], surface: str, examples: list[dict[str, str]]) -> tuple[list[dict[str, str]], dict[str, object]]:
    selected: list[dict[str, str]] = []
    select_all = False
    unknown: list[str] = []
    firmware_paths: list[str] = []
    release_review_paths: list[str] = []
    non_docs = 0
    for changed in paths:
        changed = changed.strip("/")
        lower = changed.lower()
        if not changed:
            continue
        if lower == "firmware" or lower.startswith("firmware/"):
            firmware_paths.append(changed)
            if Path(lower).suffix in ARCHIVE_SUFFIXES:
                release_review_paths.append(changed)
            continue
        if is_documentation(changed):
            continue
        non_docs += 1
        if changed.startswith(GLOBAL_PREFIXES) or changed == ".github/workflows/examples.yml":
            select_all = True
            continue
        if surface == "esp-idf" and changed.startswith("config/"):
            select_all = True
            continue
        direct = [example for example in examples if changed == example["path"] or changed.startswith(example["path"] + "/")]
        if direct:
            selected.extend(direct)
            continue
        if surface == "arduino":
            roots = {example["libraries"] for example in examples}
            matching_roots = [root for root in roots if changed == root or changed.startswith(root + "/")]
            if matching_roots:
                selected.extend(example for example in examples if example["libraries"] in matching_roots)
                continue
        unknown.append(changed)
        select_all = True
    if select_all:
        selected = examples
    else:
        selected_paths = {example["path"] for example in selected}
        selected = [example for example in examples if example["path"] in selected_paths]
    route = "all" if len(selected) == len(examples) and selected else "selected" if selected else "none"
    return selected, {"route": route, "docs_only": bool(paths) and non_docs == 0 and not firmware_paths,
                      "firmware": bool(firmware_paths), "release_review": bool(release_review_paths),
                      "unknown_paths": unknown, "changed_paths": paths}


def build_matrix(args: argparse.Namespace, selected: list[dict[str, str]]) -> dict[str, list[dict[str, str]]]:
    include: list[dict[str, str]] = []
    if args.surface == "esp-idf":
        for example in selected:
            for idf in (item.strip() for item in args.idf_versions.split(",")):
                if idf:
                    include.append({"name": example["name"], "path": example["path"], "idf": idf})
    else:
        include = [{**example, "core": args.arduino_core, "fqbn": args.fqbn} for example in selected]
    return {"include": include}


def write_github_output(path: str, matrix: dict[str, list[dict[str, str]]], evidence: dict[str, object]) -> None:
    if not path:
        return
    with open(path, "a", encoding="utf-8") as output:
        output.write(f"matrix={json.dumps(matrix, separators=(',', ':'))}\ncount={len(matrix['include'])}\n")
        for key, value in evidence.items():
            output.write(f"{key}={json.dumps(value, separators=(',', ':')) if isinstance(value, (list, dict)) else str(value).lower() if isinstance(value, bool) else value}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--surface", choices=("esp-idf", "arduino"), required=True)
    parser.add_argument("--selector", default="")
    parser.add_argument("--mode", choices=("changed", "all"), default="changed")
    parser.add_argument("--base-ref")
    parser.add_argument("--head-ref", default="HEAD")
    parser.add_argument("--changed-files-from")
    parser.add_argument("--idf-versions", default="v5.5.5,v6.0.2")
    parser.add_argument("--arduino-core", default="3.3.11")
    parser.add_argument("--fqbn", default="esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB")
    parser.add_argument("--github-output", default=os.environ.get("GITHUB_OUTPUT", ""))
    args = parser.parse_args()
    examples = all_examples(args.surface)
    selector = args.selector.replace("\\", "/").strip().strip("/")
    try:
        if args.mode == "all" or selector:
            selected = [example for example in examples if matches_selector(example, selector)]
            evidence = {"route": "all" if len(selected) == len(examples) else "selected" if selected else "none",
                        "docs_only": False, "firmware": False, "release_review": False,
                        "unknown_paths": [], "changed_paths": []}
        else:
            paths = parse_changed_file_input(Path(args.changed_files_from)) if args.changed_files_from else git_changed_paths(args.base_ref, args.head_ref)
            selected, evidence = classify_paths(paths, args.surface, examples)
    except DiffUnavailable as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    matrix = build_matrix(args, selected)
    result = {**matrix, **evidence}
    write_github_output(args.github_output, matrix, evidence)
    print(json.dumps(result, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
