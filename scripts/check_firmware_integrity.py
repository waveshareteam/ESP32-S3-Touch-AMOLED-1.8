#!/usr/bin/env python3
"""Verify the immutable identities of tracked factory firmware binaries."""

from __future__ import annotations

import argparse
import hashlib
import json
import stat
import subprocess
import sys
from pathlib import Path, PurePosixPath


def safe_firmware_path(value: object) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError("firmware path must be a non-empty repository-relative POSIX path")
    path = PurePosixPath(value)
    if (path.is_absolute() or len(path.parts) != 2 or path.parts[:1] != ("Firmware",)
            or path.suffix != ".bin"
            or any(part in {"", ".", ".."} for part in path.parts)):
        raise ValueError("firmware path must name a .bin directly under the case-sensitive Firmware root")
    return path.as_posix()


def load_manifest(path: Path) -> list[dict[str, object]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict) or set(data) != {"firmware"} or not isinstance(data["firmware"], list):
        raise ValueError("manifest must contain only a firmware list")
    entries: list[dict[str, object]] = []
    seen: set[str] = set()
    for item in data["firmware"]:
        if not isinstance(item, dict) or set(item) != {"path", "size", "sha256"}:
            raise ValueError("each firmware entry must contain only path, size, and sha256")
        relative = safe_firmware_path(item["path"])
        if relative in seen:
            raise ValueError("firmware paths must be unique")
        if not isinstance(item["size"], int) or isinstance(item["size"], bool) or item["size"] < 0:
            raise ValueError("firmware size must be a non-negative integer")
        digest = item["sha256"]
        if not isinstance(digest, str) or len(digest) != 64 or any(char not in "0123456789abcdef" for char in digest):
            raise ValueError("firmware sha256 must be a lowercase 64-character hexadecimal digest")
        seen.add(relative)
        entries.append({"path": relative, "size": item["size"], "sha256": digest})
    if not entries:
        raise ValueError("firmware list must not be empty")
    return entries


def git_root(cwd: Path) -> Path:
    try:
        completed = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"], cwd=cwd, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise RuntimeError("unable to determine the repository root") from exc
    return Path(completed.stdout.decode("utf-8", "surrogateescape").strip())


def manifest_at_root(repo: Path, requested: Path) -> Path:
    candidate = requested if requested.is_absolute() else Path.cwd() / requested
    candidate = candidate.absolute()
    expected = repo / "firmware_integrity.json"
    if candidate != expected:
        raise ValueError("manifest must be firmware_integrity.json at the repository root")
    metadata = candidate.lstat()
    if not stat.S_ISREG(metadata.st_mode) or candidate.is_symlink():
        raise ValueError("manifest must be a regular non-symlink file")
    return candidate


def tracked_firmware_bins(repo: Path, revision: str | None = None) -> set[str]:
    command = (["git", "ls-tree", "-r", "-z", "--name-only", revision, "--", "Firmware"]
               if revision else ["git", "ls-files", "-z", "--", "Firmware"])
    try:
        completed = subprocess.run(
            command, cwd=repo, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        scope = f" at {revision}" if revision else ""
        raise RuntimeError(f"unable to enumerate tracked Firmware files{scope}") from exc
    return {item for item in completed.stdout.decode("utf-8", "surrogateescape").split("\0")
            if item.startswith("Firmware/") and item.lower().endswith(".bin")}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def revision_blob_identity(repo: Path, revision: str, relative: str) -> tuple[int, str]:
    try:
        completed = subprocess.run(
            ["git", "show", f"{revision}:{relative}"], cwd=repo, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise RuntimeError(f"unable to read immutable firmware from {revision}: {relative}") from exc
    return len(completed.stdout), hashlib.sha256(completed.stdout).hexdigest()


def check(repo: Path, manifest: list[dict[str, object]], base_ref: str | None = None) -> list[str]:
    findings: list[str] = []
    listed = {entry["path"] for entry in manifest}
    try:
        tracked = tracked_firmware_bins(repo)
    except RuntimeError as exc:
        return [str(exc)]
    for path in sorted(tracked - listed):
        findings.append(f"UNLISTED_TRACKED_FIRMWARE: {path}")
    for entry in manifest:
        relative = str(entry["path"])
        path = repo / relative
        if relative not in tracked:
            findings.append(f"UNTRACKED_FIRMWARE: {relative}")
            continue
        try:
            metadata = path.lstat()
        except OSError:
            findings.append(f"MISSING_FIRMWARE: {relative}")
            continue
        if not stat.S_ISREG(metadata.st_mode) or path.is_symlink():
            findings.append(f"UNSAFE_FIRMWARE: {relative}")
            continue
        if metadata.st_size != entry["size"]:
            findings.append(f"SIZE_MISMATCH: {relative}")
            continue
        try:
            actual = sha256(path)
        except OSError:
            findings.append(f"UNREADABLE_FIRMWARE: {relative}")
            continue
        if actual != entry["sha256"]:
            findings.append(f"SHA256_MISMATCH: {relative}")
    if base_ref:
        try:
            base_tracked = tracked_firmware_bins(repo, base_ref)
        except RuntimeError as exc:
            return findings + [str(exc)]
        for relative in sorted(tracked - base_tracked):
            findings.append(f"IMMUTABLE_FIRMWARE_ADDED: {relative}")
        for relative in sorted(base_tracked - tracked):
            findings.append(f"IMMUTABLE_FIRMWARE_REMOVED: {relative}")
        for relative in sorted(tracked & base_tracked):
            path = repo / relative
            try:
                metadata = path.lstat()
                if not stat.S_ISREG(metadata.st_mode) or path.is_symlink():
                    continue
                base_size, base_digest = revision_blob_identity(repo, base_ref, relative)
                if metadata.st_size != base_size or sha256(path) != base_digest:
                    findings.append(f"IMMUTABLE_FIRMWARE_CHANGED: {relative}")
            except (OSError, RuntimeError) as exc:
                findings.append(str(exc))
    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", default="firmware_integrity.json")
    parser.add_argument("--base-ref", help="trusted Git revision whose factory binaries must remain unchanged")
    args = parser.parse_args()
    try:
        repo = git_root(Path.cwd())
        config = manifest_at_root(repo, Path(args.manifest))
        manifest = load_manifest(config)
        findings = check(repo, manifest, args.base_ref)
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as exc:
        print(f"CONFIG_ERROR: {exc}", file=sys.stderr)
        return 2
    for finding in findings:
        print(finding)
    if findings:
        return 1
    print(f"PASS: verified {len(manifest)} immutable factory firmware identities")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
