#!/usr/bin/env python3
"""Discover ESP-IDF examples that should be built by CI."""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
import subprocess
import sys
from pathlib import Path


EXAMPLES_ROOT = Path("examples/esp-idf")
DEFAULT_EXAMPLES = ("00_board_check", "00_bsp_quickstart")
GLOBAL_EXAMPLE_PATTERNS = (
    ".github/workflows/esp-idf-examples.yml",
    ".github/scripts/discover_esp_idf_examples.py",
)


def run_git(args: list[str]) -> list[str]:
    result = subprocess.run(
        ["git", *args],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def list_examples() -> list[str]:
    if not EXAMPLES_ROOT.exists():
        return []

    examples = []
    for path in EXAMPLES_ROOT.iterdir():
        if (path / "CMakeLists.txt").is_file() and (path / "main").is_dir():
            examples.append(path.as_posix())
    return sorted(examples)


def default_examples(known_examples: set[str]) -> list[str]:
    selected = []
    for name in DEFAULT_EXAMPLES:
        example = (EXAMPLES_ROOT / name).as_posix()
        if example in known_examples:
            selected.append(example)
    return selected


def normalize_example(value: str) -> str:
    value = value.strip().strip("/")
    if not value:
        return value
    if value == "all":
        return value
    if value.startswith(EXAMPLES_ROOT.as_posix() + "/"):
        return value
    return (EXAMPLES_ROOT / value).as_posix()


def discover_from_paths(paths: list[str], known_examples: set[str]) -> list[str]:
    selected = set()
    root_prefix = EXAMPLES_ROOT.as_posix() + "/"

    for changed_path in paths:
        changed_path = changed_path.strip().strip("/")
        if any(fnmatch.fnmatch(changed_path, pattern) for pattern in GLOBAL_EXAMPLE_PATTERNS):
            selected.update(default_examples(known_examples))
            continue

        if not changed_path.startswith(root_prefix):
            continue

        parts = Path(changed_path).parts
        if len(parts) < 3:
            selected.update(default_examples(known_examples))
            continue

        example = Path(*parts[:3]).as_posix()
        if example in known_examples:
            selected.add(example)

    selected.update(default_examples(known_examples))
    return sorted(selected)


def discover_changed_examples(base_ref: str | None, head_ref: str, known_examples: set[str]) -> list[str]:
    if base_ref:
        diff_args = ["diff", "--name-only", f"{base_ref}...{head_ref}"]
    else:
        diff_args = ["diff-tree", "--no-commit-id", "--name-only", "-r", head_ref]

    return discover_from_paths(run_git(diff_args), known_examples)


def github_output(name: str, value: str) -> None:
    output_path = os.environ.get("GITHUB_OUTPUT")
    if output_path:
        with open(output_path, "a", encoding="utf-8") as output:
            output.write(f"{name}={value}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-ref")
    parser.add_argument("--head-ref", default="HEAD")
    parser.add_argument("--example", default="")
    parser.add_argument(
        "--fallback-default",
        action="store_true",
        help="Build the default smoke-test examples when no changed example is detected.",
    )
    args = parser.parse_args()

    known_examples = set(list_examples())
    requested_example = normalize_example(args.example)

    if requested_example == "all":
        selected = sorted(known_examples)
    elif requested_example:
        if requested_example not in known_examples:
            print(f"Unknown ESP-IDF example: {args.example}", file=sys.stderr)
            print("Known examples:", file=sys.stderr)
            for example in sorted(known_examples):
                print(f"  {example}", file=sys.stderr)
            return 1
        selected = [requested_example]
    else:
        selected = discover_changed_examples(args.base_ref, args.head_ref, known_examples)
        if args.fallback_default and not selected:
            selected = default_examples(known_examples)

    matrix = {"example": selected}
    matrix_json = json.dumps(matrix, separators=(",", ":"))
    has_examples = "true" if selected else "false"

    github_output("matrix", matrix_json)
    github_output("has_examples", has_examples)
    github_output("examples", ",".join(selected))

    print(matrix_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
