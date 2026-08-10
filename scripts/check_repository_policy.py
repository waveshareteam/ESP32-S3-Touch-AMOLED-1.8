#!/usr/bin/env python3
"""Limited first-party documentation policy; not a complete documentation audit."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


LINK_RE = re.compile(r"(?<!!)\[[^]]*\]\(([^)\s]+)(?:\s+[^)]*)?\)")
DRIVE_RE = re.compile(r"(?<![A-Za-z])[A-Za-z]:[\\/]")
UNC_RE = re.compile(r"(?<!\S)\\\\[^\\/]+[\\/]")
PROVENANCE_RE = re.compile(r"\b(?:ChatGPT|Codex|GPT-[0-9]|Claude|Gemini)\b", re.IGNORECASE)


def load_config(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data.get("bilingual_pairs"), list):
        raise ValueError("bilingual_pairs must be a list")
    return data


def under_prefix(path: Path, prefix: str) -> bool:
    return prefix == "." or path.as_posix() == prefix or path.as_posix().startswith(prefix.rstrip("/") + "/")


def first_party_markdown(repo: Path, config: dict) -> list[Path]:
    roots = config.get("first_party_roots", ["."])
    excluded = config.get("exclude_prefixes", [])
    return [path for path in repo.rglob("*.md")
            if not any(under_prefix(path.relative_to(repo), prefix) for prefix in excluded)
            and any(under_prefix(path.relative_to(repo), prefix) for prefix in roots)]


def relative_targets(text: str) -> list[str]:
    targets = []
    for target in LINK_RE.findall(text):
        target = target.split("#", 1)[0].strip("<>")
        if target and not re.match(r"(?:[a-z]+:|#|/)", target, re.I):
            targets.append(target)
    return targets


def links_to(source: Path, target: Path) -> bool:
    return any((source.parent / link).resolve() == target.resolve()
               for link in relative_targets(source.read_text(encoding="utf-8")))


def check(repo: Path, config: dict) -> list[str]:
    findings: list[str] = []
    for pair in config["bilingual_pairs"]:
        english, chinese = repo / pair["english"], repo / pair["chinese"]
        if not english.is_file() or not chinese.is_file():
            findings.append(f"BILINGUAL_PAIR_MISSING: {pair['english']} <-> {pair['chinese']}")
            continue
        if not links_to(english, chinese):
            findings.append(f"BILINGUAL_LINK_MISSING: {pair['english']} -> {pair['chinese']}")
        if not links_to(chinese, english):
            findings.append(f"BILINGUAL_LINK_MISSING: {pair['chinese']} -> {pair['english']}")
    for path in first_party_markdown(repo, config):
        text = path.read_text(encoding="utf-8")
        relative = path.relative_to(repo).as_posix()
        if DRIVE_RE.search(text) or UNC_RE.search(text):
            findings.append(f"LOCAL_PATH: {relative}")
        if PROVENANCE_RE.search(text):
            findings.append(f"TOOL_OR_MODEL_PROVENANCE: {relative}")
        for target in relative_targets(text):
            candidate = (path.parent / target).resolve()
            try:
                candidate.relative_to(repo.resolve())
            except ValueError:
                findings.append(f"RELATIVE_LINK_ESCAPES_REPOSITORY: {relative} -> {target}")
                continue
            if not candidate.exists():
                findings.append(f"RELATIVE_LINK_MISSING: {relative} -> {target}")
    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", default=".")
    parser.add_argument("--config", default="repository_policy.json")
    args = parser.parse_args()
    try:
        findings = check(Path(args.repo).resolve(), load_config(Path(args.config)))
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    for finding in findings:
        print(finding)
    return 1 if findings else 0


if __name__ == "__main__":
    raise SystemExit(main())
