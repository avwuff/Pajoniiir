"""Exercise the private overlay generator and configure guards, without hardware."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import prepare_jc8012_ppa as tool

CMAKE = shutil.which('cmake')
if not CMAKE:
    candidates = sorted(Path('C:/Espressif/tools/cmake').glob('*/bin/cmake.exe'))
    if not candidates:
        raise RuntimeError('CMake is required for private PPA isolation tests')
    CMAKE = str(candidates[-1])


class PreparationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.base = Path(self.temp.name)
        self.idf, self.spec = self.base/'idf', self.base/'spec'
        self.spec.mkdir()
        self.source = self.idf/'components/esp_driver_ppa'
        original = json.loads((tool.SPEC/'pins.json').read_text())
        self.files = {name: f'fixture {name}\n' for name in original['idf_files']}
        self.files['CMakeLists.txt'] = '# fixture CMakeLists.txt\n'
        self.files['src/ppa_srm.c'] = 'prefix\nold workaround\nsuffix\n'
        for name, data in self.files.items():
            p = self.source/name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(data)
        (self.idf/'LICENSE').write_text('fixture licence\n')
        self.pins = {'donor_sha': original['donor_sha'],
                     'idf_files': {name: tool.digest(data) for name, data in self.files.items()},
                     'donor_srm_sha256': tool.digest('prefix\nnew workaround\nsuffix\n')}
        self.write_pins()
        (self.spec/'workaround.before').write_text('old workaround\n')
        (self.spec/'workaround.after').write_text('new workaround\n')
        self.override = patch.object(tool, 'SPEC', self.spec)
        self.override.start()

    def tearDown(self):
        self.override.stop()
        self.temp.cleanup()

    def write_pins(self):
        (self.spec/'pins.json').write_text(json.dumps(self.pins))

    def generate(self, output='out', board='jc8012p4a1'):
        return tool.prepare(self.idf, self.base/output, board)

    def test_generate_leaves_idf_intact_and_seals_output(self):
        result = self.generate()
        for name, original in self.files.items():
            self.assertEqual((self.source/name).read_text(), original)
        for name, expected in result['files_sha256'].items():
            self.assertEqual(tool.digest((self.base/'out'/name).read_text()), expected)
        self.assertEqual((self.base/'out/src/ppa_srm.c').read_text(), 'prefix\nnew workaround\nsuffix\n')

    def test_drift_in_any_pinned_file_is_rejected_before_writing(self):
        for name in self.files:
            with self.subTest(name=name):
                p = self.source/name
                p.write_text('drift')
                with self.assertRaisesRegex(ValueError, 'source drift'):
                    self.generate()
                self.assertFalse((self.base/'out').exists())
                p.write_text(self.files[name])

    def test_wrong_board_and_existing_output_rejected(self):
        for board in ('m3', 'jc4880', 'jc1060', ''):
            with self.assertRaisesRegex(ValueError, 'restricted'):
                self.generate(board=board)
        self.generate()
        with self.assertRaisesRegex(ValueError, 'new directory'):
            self.generate()
        with self.assertRaisesRegex(ValueError, 'outside ESP-IDF'):
            tool.prepare(self.idf, self.idf/'new', 'jc8012p4a1')

    def test_bad_anchor_and_wrong_result_hash_rejected(self):
        (self.spec/'workaround.before').write_text('missing')
        with self.assertRaisesRegex(ValueError, 'anchor'):
            self.generate()
        (self.spec/'workaround.before').write_text('old workaround\n')
        (self.spec/'workaround.after').write_text('incorrect patch\n')
        with self.assertRaisesRegex(ValueError, 'frozen donor'):
            self.generate()

    def test_generated_cmake_refuses_other_boards(self):
        self.generate()
        for board in ('M3', 'JC4880', 'JC1060'):
            script = self.base/'check.cmake'
            script.write_text(f'set(CONFIG_PAJONIIIR_BOARD_{board} 1)\n'
                              f'include("{(self.base/"out/CMakeLists.txt").as_posix()}")\n')
            run = subprocess.run([CMAKE, '-P', str(script)], capture_output=True, text=True)
            self.assertNotEqual(run.returncode, 0)
            self.assertIn('JC8012 private PPA selected by another board', run.stderr)

    def test_selection_helper_refuses_wrong_board(self):
        script = self.base/'select.cmake'
        script.write_text(f'include("{(ROOT/"firmware/boards/jc8012p4a1/ppa/select.cmake").as_posix()}")\n'
                          'pajoniiir_prepare_jc8012_ppa(m3 output)\n')
        run = subprocess.run([CMAKE, '-P', str(script)], capture_output=True, text=True)
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('requires jc8012p4a1', run.stderr)


if __name__ == '__main__':
    unittest.main()
