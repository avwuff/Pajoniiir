"""Build the production identity component incrementally in an isolated Git repo."""
from pathlib import Path
import json
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
def tool(name):
    found=shutil.which(name)
    if found: return found
    matches=sorted(Path('C:/Espressif/tools',name).glob('*/bin/'+name+'.exe'))
    if not matches: matches=sorted(Path('C:/Espressif/tools',name).glob('*/'+name+'.exe'))
    if not matches: raise RuntimeError(name+' required')
    return str(matches[-1])
CMAKE=tool('cmake')
NINJA=tool('ninja')

class IdentityTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.repo=Path(self.temp.name)
        self.build=self.repo/'build'
        adapter=self.repo/'firmware/p4-core/components/board_adapter'
        adapter.mkdir(parents=True)
        original=ROOT/'firmware/p4-core/components/board_adapter'
        for name in ['board_build_identity.c','board_build_identity_generated.h.in', 'board_build_identity.json.in']:
            shutil.copyfile(original/name,adapter/name)
        (adapter/'include').mkdir()
        shutil.copyfile(original/'include/board_build_identity.h',adapter/'include/board_build_identity.h')
        cmake_dir=adapter.parent.parent/'cmake';cmake_dir.mkdir()
        shutil.copyfile(ROOT/'firmware/p4-core/cmake/generate_board_identity.cmake',cmake_dir/'generate_board_identity.cmake')
        tail=(original/'CMakeLists.txt').read_text().split('find_package(Git REQUIRED)',1)[1]
        (adapter/'CMakeLists.txt').write_text('find_package(Git REQUIRED)'+tail)
        (self.repo/'.gitignore').write_text('/build/\n')
        (self.repo/'probe.c').write_text('#include <stdio.h>\n#include "board_build_identity.h"\nint main(void){printf("%s %d",board_build_source_sha(),board_build_source_dirty());}\n')
        (self.repo/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.22)
project(identity_fixture C)
add_library(identity firmware/p4-core/components/board_adapter/board_build_identity.c)
set(COMPONENT_LIB identity)
add_subdirectory(firmware/p4-core/components/board_adapter)
target_include_directories(identity PUBLIC firmware/p4-core/components/board_adapter/include)
add_executable(probe probe.c)
target_link_libraries(probe PRIVATE identity)
''')
        self.git('init','-q');self.commit()

    def tearDown(self): self.temp.cleanup()
    def git(self,*args):
        return subprocess.check_output(['git','-C',str(self.repo),*args],text=True).strip()
    def commit(self):
        self.git('add','.')
        self.git('-c','user.email=fixture@example.invalid','-c','user.name=Fixture','commit','-qm','fixture')
        return self.git('rev-parse','HEAD')
    def configure(self,release=False):
        subprocess.run([CMAKE,'-S',str(self.repo),'-B',str(self.build),'-G','Ninja',
            '-DCMAKE_MAKE_PROGRAM='+NINJA,'-DCMAKE_C_COMPILER='+str(shutil.which('gcc')),
            '-DPAJONIIIR_RELEASE_BUILD='+('ON' if release else 'OFF')],check=True,capture_output=True)
    def run_build(self,success=True):
        result=subprocess.run([CMAKE,'--build',str(self.build)],capture_output=True,text=True)
        self.assertEqual(result.returncode==0,success,result.stdout+result.stderr)
        return result
    def identity(self):
        result=json.loads((self.build/'board_build_identity.json').read_text())
        probe=self.build/('probe.exe' if os.name=='nt' else 'probe')
        self.assertEqual(subprocess.check_output([str(probe)],text=True),
            result['source_sha']+' '+str(result['source_dirty']))
        return result

    def test_incremental_dirty_commit_and_noop(self):
        sha=self.git('rev-parse','HEAD');self.configure();self.run_build()
        self.assertEqual(self.identity(),{'source_sha':sha,'source_dirty':0})
        header=self.build/'firmware/p4-core/components/board_adapter/board_build_identity_generated.h'
        before=header.stat().st_mtime_ns;self.run_build()
        self.assertEqual(header.stat().st_mtime_ns,before)
        (self.repo/'changed-source.txt').write_text('working-tree change')
        self.run_build()  # no configure call
        self.assertEqual(self.identity(),{'source_sha':sha,'source_dirty':1})
        sha=self.commit();self.run_build()  # no configure call
        self.assertEqual(self.identity(),{'source_sha':sha,'source_dirty':0})

    def test_release_rejects_changes_after_configure(self):
        self.configure(release=True);self.run_build()
        (self.repo/'changed-source.txt').write_text('dirty after configure')
        result=self.run_build(success=False)
        self.assertIn('clean committed source',result.stdout+result.stderr)

if __name__=='__main__': unittest.main()
