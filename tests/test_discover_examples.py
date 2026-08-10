"""Synthetic CLI coverage for the GitHub Actions routing invocation."""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
SCRIPT = REPO / "scripts" / "discover_examples.py"
IDF_NAMES = ["00_board_check", "00_bsp_quickstart", "01_project_template", "02_hello_world",
             "03_nvs_counter", "04_freertos_tasks", "05_gpio_io", "06_gpio_interrupt", "08_i2c_tools",
             "09_sdmmc", "10_wifi_station", "12_i2s_codec", "13_display_colorbar", "14_lvgl_demo_v9",
             "90_axp2101_pmu", "91_pcf85063_rtc", "92_qmi8658_imu"]
ARDUINO_NAMES = ["arduino-01_HelloWorld", "arduino-02_Drawing_board", "arduino-03_GFX_AsciiTable",
                 "arduino-04_GFX_FT3168_Image", "arduino-05_GFX_PCF85063_simpleTime",
                 "arduino-06_GFX_ESPWiFiAnalyzer", "arduino-07_GFX_Clock", "arduino-08_LVGL_Animation-ui",
                 "arduino-09_LVGL_change_background-ui", "arduino-10_LVGL_PCF85063_simpleTime",
                 "arduino-11_LVGL_QMI8658_ui", "arduino-12_LVGL_AXP2101_ADC_Data",
                 "arduino-13_LVGL_Widgets", "arduino-14_LVGL_SD_Test", "arduino-15_ES8311",
                 "arduino-16_LVGL_Sqprj", "arduino-v2-01_HelloWorld", "arduino-v2-02_Drawing_board",
                 "arduino-v2-03_GFX_AsciiTable", "arduino-v2-04_GFX_FT3168_Image",
                 "arduino-v2-05_GFX_PCF85063_simpleTime", "arduino-v2-09_LVGL_change_background-ui",
                 "arduino-v2-11_LVGL_QMI8658_ui", "arduino-v2-13_LVGL_Widgets",
                 "arduino-v2-14_LVGL_SD_Test", "arduino-v2-15_ES8311"]


class DiscoverExamplesCliTests(unittest.TestCase):
    def invoke(self, surface: str, paths: str | None = None, *extra: str) -> subprocess.CompletedProcess[str]:
        command = [sys.executable, "-B", str(SCRIPT), "--surface", surface, *extra]
        if paths is not None:
            fixture = tempfile.NamedTemporaryFile("w", encoding="utf-8", delete=False)
            self.addCleanup(lambda: Path(fixture.name).unlink(missing_ok=True))
            fixture.write(paths)
            fixture.close()
            command.extend(["--changed-files-from", fixture.name])
        return subprocess.run(command, cwd=REPO, text=True, capture_output=True, check=False)

    def result(self, surface: str, paths: str, *extra: str) -> dict:
        completed = self.invoke(surface, paths, *extra)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        return json.loads(completed.stdout)

    def names(self, result: dict) -> list[str]:
        return [row["name"] for row in result["include"]]

    def test_full_inventory_names_and_matrix_shapes(self) -> None:
        idf = self.result("esp-idf", "ignored.md\n", "--selector", "all")
        arduino = self.result("arduino", "ignored.md\n", "--mode", "all")
        self.assertEqual(self.names(idf)[::2], IDF_NAMES)
        self.assertEqual(len(idf["include"]), 34)
        self.assertEqual(self.names(arduino), ARDUINO_NAMES)
        self.assertEqual(len(arduino["include"]), 26)

    def test_documentation_only_paths_never_select_examples(self) -> None:
        cases = ("README.md\n", "examples/esp-idf/00_board_check/README.md\n",
                 "examples/arduino/examples/01_HelloWorld/README.md\n",
                 "examples/arduino/libraries/lvgl/README.md\n")
        for paths in cases:
            with self.subTest(paths=paths):
                for surface in ("esp-idf", "arduino"):
                    result = self.result(surface, paths)
                    self.assertEqual(result["route"], "none")
                    self.assertTrue(result["docs_only"])

    def test_direct_and_shared_inputs_route_expected_surface(self) -> None:
        idf = self.result("esp-idf", "examples/esp-idf/00_board_check/main/board_check_main.c\n")
        self.assertEqual(self.names(idf), ["00_board_check", "00_board_check"])
        arduino = self.result("arduino", "examples/arduino/examples/01_HelloWorld/01_HelloWorld.ino\n")
        self.assertEqual(self.names(arduino), ["arduino-01_HelloWorld"])
        self.assertEqual(len(self.result("esp-idf", "config/toolchain.txt\n")["include"]), 34)
        self.assertEqual(len(self.result("arduino", "examples/arduino/libraries/lvgl/src/lv_obj.c\n")["include"]), 16)
        self.assertEqual(len(self.result("esp-idf", ".github/workflows/examples.yml\n")["include"]), 34)
        self.assertEqual(len(self.result("arduino", "tests/test_discover_examples.py\n")["include"]), 26)

    def test_firmware_unknown_and_rename_deletion_evidence(self) -> None:
        for firmware_path in ("Firmware/notes.md", "FIRMWARE/source/main.c", "Firmware/image.bin", "firmware/release.zip"):
            result = self.result("esp-idf", firmware_path + "\n")
            self.assertEqual(result["route"], "none")
            self.assertTrue(result["firmware"])
            self.assertFalse(result["docs_only"])
        release = self.result("arduino", "Firmware/image.bin\n")
        self.assertTrue(release["release_review"])
        unknown = self.result("esp-idf", "new-build-input.toml\n")
        self.assertEqual(unknown["route"], "all")
        self.assertEqual(unknown["unknown_paths"], ["new-build-input.toml"])
        rename = self.result("esp-idf", "R100\texamples/esp-idf/00_board_check/main/old.c\texamples/esp-idf/00_board_check/main/new.c\n")
        self.assertEqual(self.names(rename), ["00_board_check", "00_board_check"])
        deleted = self.result("arduino", "D\texamples/arduino/examples/01_HelloWorld/01_HelloWorld.ino\n")
        self.assertEqual(self.names(deleted), ["arduino-01_HelloWorld"])

    def test_empty_or_unavailable_changed_input_fails_closed(self) -> None:
        self.assertEqual(self.invoke("esp-idf", "").returncode, 2)
        missing = subprocess.run([sys.executable, "-B", str(SCRIPT), "--surface", "arduino",
                                  "--changed-files-from", "does-not-exist"], cwd=REPO, text=True,
                                 capture_output=True, check=False)
        self.assertEqual(missing.returncode, 2)
        no_diff = subprocess.run([sys.executable, "-B", str(SCRIPT), "--surface", "esp-idf"], cwd=REPO,
                                 text=True, capture_output=True, check=False)
        self.assertEqual(no_diff.returncode, 2)

    def test_workflow_base_head_versions_fqbn_and_github_output(self) -> None:
        """Exercise the workflow's git-diff CLI mode in small independent repositories."""
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "scripts").mkdir()
            shutil.copy2(SCRIPT, root / "scripts" / "discover_examples.py")
            self.git(root, "init")
            self.git(root, "config", "user.email", "ci@example.invalid")
            self.git(root, "config", "user.name", "CI Test")
            self.assert_workflow_git_route(root, "esp-idf")
            self.assert_workflow_git_route(root, "arduino")

    def git(self, root: Path, *args: str) -> str:
        return subprocess.run(["git", *args], cwd=root, text=True, capture_output=True, check=True).stdout.strip()

    def assert_workflow_git_route(self, root: Path, surface: str) -> None:
        if surface == "esp-idf":
            project = root / "examples" / "esp-idf" / "00_sample"
            (project / "main").mkdir(parents=True)
            (project / "CMakeLists.txt").write_text("cmake_minimum_required(VERSION 3.16)\n", encoding="utf-8")
            old, new = project / "main" / "old.c", project / "main" / "new.c"
            old.write_text("int app_main(void) { return 0; }\n", encoding="utf-8")
            untouched = root / "examples" / "esp-idf" / "01_untouched"
            (untouched / "main").mkdir(parents=True)
            (untouched / "CMakeLists.txt").write_text("cmake_minimum_required(VERSION 3.16)\n", encoding="utf-8")
        else:
            project = root / "examples" / "arduino" / "examples" / "01_Sample"
            (project.parent.parent / "libraries").mkdir(parents=True)
            project.mkdir(parents=True)
            old, new = project / "01_Sample.ino", project / "02_Sample.ino"
            old.write_text("void setup() {}\nvoid loop() {}\n", encoding="utf-8")
            untouched = root / "examples" / "arduino" / "examples" / "02_Untouched"
            untouched.mkdir(parents=True)
            (untouched / "02_Untouched.ino").write_text("void setup() {}\nvoid loop() {}\n", encoding="utf-8")
        self.git(root, "add", ".")
        self.git(root, "commit", "-m", f"base {surface}")
        base = self.git(root, "rev-parse", "HEAD")
        self.git(root, "mv", str(old.relative_to(root)), str(new.relative_to(root)))
        self.git(root, "commit", "-m", f"rename {surface}")
        output = root / f"{surface}.out"
        command = [sys.executable, "-B", "scripts/discover_examples.py", "--surface", surface,
                   "--base-ref", base, "--head-ref", "HEAD", "--github-output", str(output)]
        if surface == "esp-idf":
            command.extend(("--idf-versions", "v5.5.5,v6.0.2"))
            expected_count = 2
        else:
            command.extend(("--arduino-core", "3.3.11", "--fqbn",
                            "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"))
            expected_count = 1
        completed = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        result = json.loads(completed.stdout)
        self.assertEqual(result["route"], "selected")
        self.assertEqual(len(result["include"]), expected_count)
        self.assertIn(old.relative_to(root).as_posix(), result["changed_paths"])
        self.assertIn(new.relative_to(root).as_posix(), result["changed_paths"])
        output_values = dict(line.split("=", 1) for line in output.read_text(encoding="utf-8").splitlines())
        self.assertEqual(output_values["count"], str(expected_count))
        for key in ("matrix", "route", "docs_only", "firmware", "release_review", "unknown_paths", "changed_paths"):
            self.assertIn(key, output_values)


if __name__ == "__main__":
    unittest.main()
