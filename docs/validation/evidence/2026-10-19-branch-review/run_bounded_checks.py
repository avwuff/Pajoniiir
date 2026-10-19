"""Read frozen Git blobs, export to ignored cache, execute only focused review checks."""
import hashlib,importlib.util,json,re,subprocess
from pathlib import Path
ROOT=Path.cwd();OUT=Path(__file__).resolve().parent;EXPORT=OUT/'ui-source';EXPORT.mkdir(exist_ok=True)
UI='b8dccf260db4c919cae209091a70ef18f89f18cc';NUMARK='86d6b412b2ba11908b48518dfaf26d51f6c3e0f6'
def blob(ref,path):return subprocess.check_output(['git','show',ref+':'+path])
paths=subprocess.check_output(['git','ls-tree','-r','--name-only',UI],text=True).splitlines()
for path in paths:
 if path.startswith(('firmware/p4-core/components/ui_next/','tests/ui_next_')):
  dest=EXPORT/path;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(blob(UI,path))
runner=blob(UI,'tests/run_p4_host_tests.ps1').decode()
pattern=re.compile(r'Name = "(ui_next_[^"]+)"\s+Dir = "([^"]+)"\s+Target = "([^"]+)"\s+Args = @\((.*?)\)\s*}',re.S)
tests=[]
for name,directory,target,args in pattern.findall(runner):
 arguments=re.findall(r'"([^"]*)"',args)
 cwd=EXPORT/directory;exe=cwd/target
 compile=subprocess.run(['C:/msys64/ucrt64/bin/gcc.exe',*arguments],cwd=cwd,capture_output=True,text=True)
 result={'name':name,'compile_exit':compile.returncode,'compile_output':compile.stdout+compile.stderr}
 if compile.returncode==0:
  run=subprocess.run([str(exe)],cwd=cwd,capture_output=True,text=True);result.update(run_exit=run.returncode,run_output=run.stdout+run.stderr)
 tests.append(result);print(name,result.get('run_exit',compile.returncode),flush=True)
assert len(tests)==8
controller=OUT/'numark-source';controller.mkdir(exist_ok=True)
for name in ('profile.json','profile.s3bin','README.md','numark-dj2go2-touch-profile.patch'):
 (controller/name).write_bytes(blob(NUMARK,'controllers/numark_dj2go2_touch/'+name))
spec=importlib.util.spec_from_file_location('compiler',ROOT/'tools/controller_profile/compile_profile.py')
compiler=importlib.util.module_from_spec(spec);spec.loader.exec_module(compiler)
compiled=compiler.compile_profile(json.loads((controller/'profile.json').read_text()))
assert compiled==(controller/'profile.s3bin').read_bytes() and len(compiled)==1999
all_numark=subprocess.check_output(['git','ls-tree','-r','--name-only',NUMARK],text=True).splitlines()
missing=[p for p in ('tests/controller_profile/test_numark_dj2go2_touch.c',) if p not in all_numark]
patch=(controller/'numark-dj2go2-touch-profile.patch').read_text()
match=re.search(r'diff --git a/tests/controller_profile/test_numark_dj2go2_touch.c.*?\n@@[^\n]*\n(.*?)(?=\ndiff --git|\Z)',patch,re.S)
assert match
test='\n'.join(line[1:] for line in match.group(1).splitlines() if line.startswith('+'))+'\n'
(controller/'test-from-embedded-patch.c').write_text(test)
result={'ui_source_sha':UI,'numark_source_sha':NUMARK,'ui_unit_checks':tests,
 'numark_current_compiler_byte_parity':True,'numark_binary_bytes':1999,
 'numark_binary_sha256':hashlib.sha256(compiled).hexdigest(),
 'numark_documented_tests_missing_from_branch':missing,
 'numark_patch_test_present':True,'scope':'Eight pure UI logic tests and Numark compiler parity only; no firmware/simulator/hardware acceptance, branch merge or working-tree source change'}
(OUT/'bounded-checks.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
assert all(t['compile_exit']==t.get('run_exit')==0 for t in tests)
print('PASS eight UI pure-module checks; Numark compiled fixture byte parity; missing branch test captured')
