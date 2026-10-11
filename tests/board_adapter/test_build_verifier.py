"""Run the complete board verifier on relocated IDF metadata fixtures."""
from pathlib import Path
import json
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'tools'))
from check_board_build import verify


class BuildVerifierTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.build = Path(self.temp.name)/'arbitrary-output'
        (self.build/'config').mkdir(parents=True)
        names = ['p4_app', 'board_adapter', 'board_capabilities', 'audio_engine',
                 'deck_core', 'library', 'ui', 'web_server', 'p4_ota',
                 'controller_profile_manager', 'controller_profile_runtime',
                 'controller_usb_host', 'usb_storage', 'usb_host_manager',
                 'audio_recorder', 'restart_diagnostics', 'dj_link_service',
                 'djlink', 'dj_link_core', 'bsp_p4_m3']
        self.description = {
            'project_name': 'main-deck-m3', 'target': 'esp32p4',
            'project_dir': '/project/firmware/main-deck-m3',
            'build_components': names,
            'build_component_paths': [
                '/project/firmware/p4-core/components/'+n if n not in
                ('djlink', 'dj_link_core', 'bsp_p4_m3') else '/project/other/'+n
                for n in names],
        }
        self.write_description()
        flags = ['LV_USE_CUSTOM_MALLOC', 'PAJONIIIR_BOARD_M3',
                 'AUDIO_ANTIALIAS_CACHE', 'BSP_PCM5102A_MAIN_OUT',
                 'PAJONIIIR_DEFAULT_WIFI_REMOTE']
        (self.build/'config/sdkconfig.h').write_text(
            '\n'.join('#define CONFIG_'+f+' 1' for f in flags))
        image = bytearray(256)
        struct.pack_into('<I', image, 32, 0xABCD5432)
        image[80:92] = b'main-deck-m3'
        image[150:190] = b'a'*40
        (self.build/'main-deck-m3.bin').write_bytes(image)
        self.identity = {'source_sha': 'a'*40, 'source_dirty': 0}
        (self.build/'board_build_identity.json').write_text(json.dumps(self.identity))

    def tearDown(self):
        self.temp.cleanup()

    def write_description(self):
        (self.build/'project_description.json').write_text(json.dumps(self.description))

    def check(self):
        with patch('check_board_build.subprocess.check_output',
                   side_effect=['a'*40+'\n', '']):
            verify(self.build, 'main-deck-m3')

    def test_relocated_build_and_container_paths(self):
        self.check()

    def test_private_component_copy_is_rejected(self):
        index = self.description['build_components'].index('library')
        self.description['build_component_paths'][index] = '/project/private/library'
        self.write_description()
        with self.assertRaisesRegex(AssertionError, 'private copy'):
            self.check()

    def test_second_bsp_is_rejected(self):
        self.description['build_components'].append('bsp_jc1060')
        self.description['build_component_paths'].append('/project/other/bsp_jc1060')
        self.write_description()
        with self.assertRaisesRegex(AssertionError, 'exactly one'):
            self.check()

    def test_stale_identity_is_still_rejected(self):
        self.identity['source_dirty'] = 1
        (self.build/'board_build_identity.json').write_text(json.dumps(self.identity))
        with self.assertRaisesRegex(AssertionError, 'stale build identity'):
            self.check()


if __name__ == '__main__':
    unittest.main()
