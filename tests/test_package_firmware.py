"""Synthetic packaging tests; no product build outputs are required."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
import zipfile
from argparse import Namespace
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("package_firmware", REPO / "releases" / "package_firmware.py")
PACKAGE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(PACKAGE)


class PackageFirmwareTests(unittest.TestCase):
    def args(self, framework: str, build: Path, output: Path) -> Namespace:
        return Namespace(framework=framework, project="examples/test", build_dir=str(build), output_dir=str(output),
                         name="test artifact", framework_version="v6", target="esp32s3", git_sha="abcdef123")

    def test_esp_idf_archive_uses_flasher_args_and_safe_layout(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root, build, output = Path(temporary), Path(temporary) / "build", Path(temporary) / "out"
            (build / "bootloader").mkdir(parents=True)
            for relative in ("bootloader/bootloader.bin", "partition-table.bin", "app.bin"):
                (build / relative).write_bytes(b"bin")
            (build / "flasher_args.json").write_text(json.dumps({"flash_files": {"0x10000": "app.bin", "0x1000": "bootloader/bootloader.bin", "0x8000": "partition-table.bin"}, "extra_esptool_args": {"before": "default_reset", "after": "hard_reset"}, "write_flash_args": ["--flash_mode", "dio"]}), encoding="utf-8")
            archive = PACKAGE.package(self.args("esp-idf", build, output))
            with zipfile.ZipFile(archive) as contents:
                names = contents.namelist()
                self.assertTrue(all(not name.startswith("/") and ".." not in Path(name).parts for name in names))
                manifest = json.loads(contents.read(next(name for name in names if name.endswith("manifest.json"))))
                self.assertEqual([item["offset"] for item in manifest["files"]], ["0x1000", "0x8000", "0x10000"])
                self.assertIn("flash.sh", "\n".join(names))
                self.assertIn("--flash_mode dio", manifest["flash_command"])

    def test_arduino_merged_and_separate_bins(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            merged = root / "merged"; merged.mkdir(); (merged / "app.merged.bin").write_bytes(b"merged")
            merged_out = root / "merged-out"
            archive = PACKAGE.package(self.args("arduino", merged, merged_out))
            with zipfile.ZipFile(archive) as contents:
                manifest = json.loads(contents.read(next(name for name in contents.namelist() if name.endswith("manifest.json"))))
                self.assertEqual([entry["offset"] for entry in manifest["files"]], ["0x0"])
            separate = root / "separate"; separate.mkdir()
            for name in ("app.bootloader.bin", "app.partitions.bin", "boot_app0.bin", "app.bin"):
                (separate / name).write_bytes(name.encode())
            separate_out = root / "separate-out"
            archive = PACKAGE.package(self.args("arduino", separate, separate_out))
            with zipfile.ZipFile(archive) as contents:
                manifest = json.loads(contents.read(next(name for name in contents.namelist() if name.endswith("manifest.json"))))
                self.assertEqual([entry["offset"] for entry in manifest["files"]], ["0x0", "0x8000", "0xe000", "0x10000"])
                self.assertIn("<PORT>", manifest["flash_command"])


if __name__ == "__main__":
    unittest.main()
