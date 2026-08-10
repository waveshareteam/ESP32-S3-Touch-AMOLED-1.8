"""Fixtures for the deliberately limited first-party documentation checker."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("repository_policy", REPO / "scripts" / "check_repository_policy.py")
POLICY = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(POLICY)


class RepositoryPolicyTests(unittest.TestCase):
    def config(self, root: Path) -> dict:
        config = {"bilingual_pairs": [{"english": "README.md", "chinese": "README_CN.md"}],
                  "first_party_roots": ["."], "exclude_prefixes": ["vendor"]}
        (root / "policy.json").write_text(json.dumps(config), encoding="utf-8")
        return POLICY.load_config(root / "policy.json")

    def test_valid_pair_and_relative_link_pass(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "README.md").write_text("[中文](README_CN.md) [guide](docs/guide.md)", encoding="utf-8")
            (root / "README_CN.md").write_text("[English](README.md)", encoding="utf-8")
            (root / "docs").mkdir(); (root / "docs" / "guide.md").write_text("ok", encoding="utf-8")
            self.assertEqual(POLICY.check(root, self.config(root)), [])

    def test_reports_pair_link_path_and_provenance_but_excludes_vendor(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); (root / "vendor").mkdir()
            (root / "README.md").write_text("[missing](missing.md) C:\\work\\x Codex", encoding="utf-8")
            (root / "README_CN.md").write_text("no reciprocal link", encoding="utf-8")
            (root / "vendor" / "README.md").write_text("Codex C:\\vendor", encoding="utf-8")
            findings = POLICY.check(root, self.config(root))
            self.assertTrue(any(item.startswith("BILINGUAL_LINK_MISSING") for item in findings))
            self.assertTrue(any(item.startswith("RELATIVE_LINK_MISSING") for item in findings))
            self.assertTrue(any(item.startswith("LOCAL_PATH") for item in findings))
            self.assertTrue(any(item.startswith("TOOL_OR_MODEL_PROVENANCE") for item in findings))
            self.assertFalse(any("vendor" in item for item in findings))


if __name__ == "__main__":
    unittest.main()
