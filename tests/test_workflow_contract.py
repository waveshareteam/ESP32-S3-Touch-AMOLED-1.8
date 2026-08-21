"""Regression checks for the exact lightweight workflow contract."""

from __future__ import annotations

import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
WORKFLOW = REPO / ".github" / "workflows" / "examples.yml"


class WorkflowContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workflow = WORKFLOW.read_text(encoding="utf-8")

    def test_exact_firmware_identity_command_is_wired(self) -> None:
        self.assertIn(
            "python3 -B scripts/check_firmware_integrity.py --manifest firmware_integrity.json",
            self.workflow,
        )
        self.assertIn('--base-ref "$FIRMWARE_BASE_REF"', self.workflow)
        self.assertIn("github.event.pull_request.base.sha", self.workflow)

    def test_scope_job_consumes_every_classifier_output(self) -> None:
        self.assertIn("change-scope:", self.workflow)
        self.assertIn("if: ${{ always() }}", self.workflow)
        for output in (
            "docs_only", "firmware", "release_review", "unknown_paths",
            "firmware_paths", "release_paths",
        ):
            self.assertIn(f"needs.discover-esp-idf.outputs.{output}", self.workflow)
        self.assertIn("needs: [discover-esp-idf, change-scope]", self.workflow)
        self.assertIn("needs: [discover-arduino, change-scope]", self.workflow)

    def test_workflow_has_no_event_path_filter_that_can_hide_scope_status(self) -> None:
        self.assertNotIn("paths-ignore:", self.workflow)
        self.assertNotIn("    paths:", self.workflow)

    def test_matrix_paths_are_passed_through_environment(self) -> None:
        self.assertIn("PROJECT_PATH: ${{ matrix.path }}", self.workflow)
        self.assertNotIn('"${{ matrix.path }}"', self.workflow)


if __name__ == "__main__":
    unittest.main()
