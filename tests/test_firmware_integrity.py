"""Synthetic CLI tests for immutable factory firmware identity checks."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
SCRIPT = REPO / "scripts" / "check_firmware_integrity.py"


class FirmwareIntegrityCliTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "Firmware").mkdir()
        subprocess.run(["git", "init", "-q"], cwd=self.root, check=True)

    def write_binary(self, name: str, content: bytes = b"factory-image") -> Path:
        path = self.root / "Firmware" / name
        path.write_bytes(content)
        subprocess.run(["git", "add", path.relative_to(self.root).as_posix()], cwd=self.root, check=True)
        return path

    def write_manifest(self, entries: list[dict[str, object]]) -> Path:
        path = self.root / "firmware_integrity.json"
        path.write_text(json.dumps({"firmware": entries}), encoding="utf-8")
        return path

    @staticmethod
    def entry(path: Path, content: bytes) -> dict[str, object]:
        return {
            "path": f"Firmware/{path.name}",
            "size": len(content),
            "sha256": hashlib.sha256(content).hexdigest(),
        }

    def invoke(self, manifest: Path, base_ref: str | None = None) -> subprocess.CompletedProcess[str]:
        command = [sys.executable, "-B", str(SCRIPT), "--manifest", str(manifest)]
        if base_ref:
            command.extend(["--base-ref", base_ref])
        return subprocess.run(
            command,
            cwd=self.root, text=True, capture_output=True, check=False,
        )

    def test_happy_path_and_tamper_fail_closed(self) -> None:
        content = b"factory-image"
        binary = self.write_binary("factory.bin", content)
        manifest = self.write_manifest([self.entry(binary, content)])
        passed = self.invoke(manifest)
        self.assertEqual(passed.returncode, 0, passed.stderr)
        binary.write_bytes(b"changed-image")
        failed = self.invoke(manifest)
        self.assertEqual(failed.returncode, 1)
        self.assertRegex(failed.stdout, r"(SIZE|SHA256)_MISMATCH: Firmware/factory.bin")

    def test_missing_and_unlisted_tracked_binary_fail(self) -> None:
        content = b"factory-image"
        binary = self.write_binary("factory.bin", content)
        manifest = self.write_manifest([self.entry(binary, content)])
        binary.unlink()
        missing = self.invoke(manifest)
        self.assertEqual(missing.returncode, 1)
        self.assertIn("MISSING_FIRMWARE: Firmware/factory.bin", missing.stdout)
        binary.write_bytes(content)
        self.write_binary("unlisted.bin", b"new-image")
        unlisted = self.invoke(manifest)
        self.assertEqual(unlisted.returncode, 1)
        self.assertIn("UNLISTED_TRACKED_FIRMWARE: Firmware/unlisted.bin", unlisted.stdout)

    def test_uppercase_extension_is_not_omitted(self) -> None:
        content = b"factory-image"
        binary = self.write_binary("factory.bin", content)
        manifest = self.write_manifest([self.entry(binary, content)])
        self.write_binary("unlisted.BIN", b"new-image")
        result = self.invoke(manifest)
        self.assertEqual(result.returncode, 1)
        self.assertIn("UNLISTED_TRACKED_FIRMWARE: Firmware/unlisted.BIN", result.stdout)

    def test_base_ref_prevents_binary_and_manifest_replacement(self) -> None:
        original = b"factory-image"
        binary = self.write_binary("factory.bin", original)
        manifest = self.write_manifest([self.entry(binary, original)])
        subprocess.run(["git", "add", "firmware_integrity.json"], cwd=self.root, check=True)
        subprocess.run(
            ["git", "-c", "user.name=Test", "-c", "user.email=test@example.invalid",
             "commit", "-q", "-m", "baseline"], cwd=self.root, check=True,
        )
        base_ref = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=self.root, text=True).strip()
        changed = b"replacement-image"
        binary.write_bytes(changed)
        self.write_manifest([self.entry(binary, changed)])
        result = self.invoke(manifest, base_ref)
        self.assertEqual(result.returncode, 1)
        self.assertIn("IMMUTABLE_FIRMWARE_CHANGED: Firmware/factory.bin", result.stdout)

    def test_unsafe_and_malformed_manifests_fail_before_hashing(self) -> None:
        unsafe = self.write_manifest([{
            "path": "Firmware/nested/factory.bin", "size": 1, "sha256": "0" * 64,
        }])
        result = self.invoke(unsafe)
        self.assertEqual(result.returncode, 2)
        self.assertIn("CONFIG_ERROR:", result.stderr)
        unsafe.write_text('{"firmware": "not-a-list"}', encoding="utf-8")
        malformed = self.invoke(unsafe)
        self.assertEqual(malformed.returncode, 2)
        self.assertIn("CONFIG_ERROR:", malformed.stderr)

    def test_manifest_symlink_is_rejected(self) -> None:
        nested = self.root / "nested"
        nested.mkdir()
        target = nested / "manifest.json"
        target.write_text('{"firmware": []}', encoding="utf-8")
        manifest = self.root / "firmware_integrity.json"
        manifest.symlink_to(target)
        result = self.invoke(manifest)
        self.assertEqual(result.returncode, 2)
        self.assertIn("regular non-symlink", result.stderr)


if __name__ == "__main__":
    unittest.main()
