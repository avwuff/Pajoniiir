from pathlib import Path
import json,hashlib,subprocess,shutil
root=Path.cwd();cache=root/'.cache/shared-cache-matrix-20261018';dst=cache/'completed-evidence';dst.mkdir(exist_ok=True)
source='2f2b05558ccb68238d7228f59e52cd6c7d06e12f'
assert subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()==source
before=json.loads((cache/'locks-before.json').read_text());after={name:hashlib.sha256((root/name).read_bytes()).hexdigest() for name in before};assert before==after
changed=subprocess.check_output(['git','diff','--name-only','1556e8d7',source],text=True).splitlines();assert all(p.startswith('docs/') for p in changed)
matrix={'source_sha':source,'production_code_identical_to_installed_m3':'1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f','build_mode':'ESP-IDF6.0.2 incremental reconfigure/build, ordinary configurations; not fresh clean builds','lock_hashes':after,'shared_host_qualification':'Passed on1556e8d7; since then documentation/evidence only','hosted_ci':'NOT RUN; user quota restriction','published':False,'targets':{}}
for target,project,build,log in (('jc4880','main-deck-p4','idf-jc4880-headphones-20261018','jc4880-build.txt'),('jc1060','main-deck-jc1060','idf-jc1060-integration-20261014','jc1060-build-retry.txt')):
    b=root/'.cache'/build;out=dst/target;out.mkdir(exist_ok=True)
    identity=json.loads((b/'board_build_identity.json').read_text());assert identity['source_sha']==source and not identity['source_dirty']
    verify=subprocess.run(['C:/Espressif/python_env/idf6.0_py3.13_env/Scripts/python.exe','tools/check_board_build.py','--build',str(b),'--project',project],capture_output=True,text=True,check=True)
    (out/'board-check.txt').write_text(verify.stdout+verify.stderr,encoding='utf-8')
    shutil.copyfile(b/'board_build_identity.json',out/'board_build_identity.json')
    raw=(cache/log).read_bytes();(out/'build-summary.txt').write_text('\n'.join(raw.decode('utf-8-sig',errors='replace').splitlines()[-65:])+'\n',encoding='utf-8')
    description=json.loads((b/'project_description.json').read_text())
    entry={'project':project,'source_sha':source,'source_dirty':False,'idf_build_exit':0,'verifier_exit':verify.returncode,'version':description['project_version'],'build_log_sha256':hashlib.sha256(raw).hexdigest(),'sdkconfig_sha256':hashlib.sha256((b/'sdkconfig').read_bytes()).hexdigest(),'physical_acceptance':'NOT RUN on this image; previous JC4880 smoke is historical and its soak remains waived; JC1060 hardware deferred to contributor'}
    for field,name in (('application',project+'.bin'),('elf',project+'.elf')):
        data=(b/name).read_bytes();entry[field+'_bytes']=len(data);entry[field+'_sha256']=hashlib.sha256(data).hexdigest()
    assert entry['application_bytes']<=0x380000
    matrix['targets'][target]=entry
matrix['targets']['m3']={'source_sha':'1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f','version':'M3-dev-g1556e8d7dfcb','build_and_host':'Previously PASS; production code identical to matrix HEAD','installed':'VALID ota_1','elf_sha256':'4999176a6b7b1fe9c0f48e7c9687a830b45873d8752afc9a8e1a61d8e7992d93','short_physical_tests':'100.264s three-loop-exit replay,30.145s isolated controls,60.133s combined semantic run pass strict telemetry and operator confirmations; evidence2026-10-18-m3-cache-bound','long_soak':'NOT RUN on this image; left last by user request'}
(dst/'.gitattributes').write_text('* -text whitespace=-blank-at-eol,-blank-at-eof,space-before-tab,cr-at-eol\n',encoding='utf-8')
shutil.copyfile(cache/'documentation-check.txt',dst/'documentation-check.txt')
shutil.copyfile(cache/'jc1060-build.txt',dst/'jc1060-cache-lock-first-attempt.txt')
shutil.copyfile(Path(__file__),dst/'audit_matrix.py')
(dst/'matrix.json').write_text(json.dumps(matrix,indent=2)+'\n',encoding='utf-8')
inventory={str(p.relative_to(dst)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(dst.rglob('*')) if p.is_file() and p.name!='sha256.json'}
(dst/'sha256.json').write_text(json.dumps(inventory,indent=2)+'\n',encoding='utf-8')
print(json.dumps(matrix,indent=2));print('Hashed matrix evidence:',len(inventory))
shutil.copytree(dst,root/'docs/validation/evidence/2026-10-18-shared-cache-builds',dirs_exist_ok=True)
