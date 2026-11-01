"""Regression for the HOME Menu crash caused by reordered CGFX DICT entries."""
import struct
import json
import sys
import unittest
from pathlib import Path

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'tools'))
from check_banner import check_cgfx


class BannerFormatTest(unittest.TestCase):
    def setUp(self):
        self.data = (root / 'assets/banner/banner.cgfx').read_bytes()

    def test_every_shipped_dictionary_name_resolves(self):
        result = check_cgfx(self.data)
        manifest=json.loads((root/'assets/banner/manifest.json').read_text())
        self.assertEqual(result['lookups_passed'], manifest['binary_checks']['lookups_passed'])

    def test_reordering_without_rebuilding_branches_is_rejected(self):
        result = check_cgfx(self.data)
        # This is what the opaque/transparent sort did in pycgfx. Move complete
        # entries, including their stale branch indices, then fix relative
        # pointers so this specifically tests name lookup, not pointer bounds.
        offset = next(d['offset'] for d in result['dictionaries'] if d['entries'] == 5)
        corrupted = bytearray(self.data)
        a, b = offset + 12 + 16, offset + 12 + 32
        for dest, source in ((a, b), (b, a)):
            entry = bytearray(self.data[source:source + 16])
            for field in (8, 12):
                relative = struct.unpack_from('<i', entry, field)[0]
                if relative:
                    struct.pack_into('<i', entry, field, relative + source - dest)
            corrupted[dest:dest + 16] = entry
        with self.assertRaisesRegex(AssertionError, 'DICT (lookup|cycle)'):
            check_cgfx(corrupted)

    def test_truncated_cgfx_is_rejected(self):
        with self.assertRaisesRegex(AssertionError, 'size mismatch'):
            check_cgfx(self.data[:-16])


if __name__ == '__main__':
    unittest.main()
