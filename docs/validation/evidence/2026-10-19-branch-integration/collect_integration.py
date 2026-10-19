import hashlib,json,re,subprocess
from pathlib import Path
root=Path.cwd(); cache=root/'.cache/branch-review-20261019'
source=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
assert source=='abffb203bfc4807b9ef622f520b02a9335afc415'
assert not subprocess.check_output(['git','status','--porcelain'],text=True).strip()
matrix=[]
for project in ('main-deck-m3','main-deck-p4','main-deck-jc1060'):
    item=json.loads((cache/f'integration-build-{project}.json').read_text(encoding='utf-8-sig'))
    assert item['build_exit']==item['verifier_exit']==0
    assert item['board_build_identity']=={'source_sha':source,'source_dirty':0}
    assert item['lock_unchanged'] and item['ui_next_selected'] is False
    assert item['ppa_component_path'].replace('\\','/').endswith('/esp-idf/components/esp_driver_ppa')
    image=(root/f'firmware/{project}/build/{project}.bin').read_bytes()
    assert len(image)==item['image_bytes']<=0x380000
    assert hashlib.sha256(image).hexdigest()==item['image_sha256'].lower()
    item['embedded_version']=image[48:80].split(b'\0')[0].decode()
    assert image[80:112].split(b'\0')[0].decode()==project
    if project=='main-deck-m3': assert item['embedded_version']=='M3-dev-g'+source[:12]
    else:
        expected=subprocess.check_output(['git','describe','--tags','--dirty','--exclude','*-g*','--match','M2.*'],text=True).strip()
        assert item['embedded_version']==expected,(project,item['embedded_version'],expected)
    matrix.append(item)
host=(cache/'integration-final-host-qualification.log').read_text(errors='replace')
assert not re.search(r'(?im)(^\s*SKIP:|^\s*\(No PDB path|SKIPPING the .*tests)',host)
summary={'source_sha':source,'host_qualification_exit':0,'host_qualification_optional_skip':False,
         'functional_build_steps':len(re.findall(r'^==> build ',host,re.M)),
         'run_steps':len(re.findall(r'^==> run ',host,re.M)),
         'focused_ui_logic_tests':8,'ui_demo_perform_render_exit':0,
         'ui_demo_perform_frames':sorted(p.name for p in (cache/'ui-next-captures').glob('*.ppm')),
         'red_green':[
             {'red':'numark-pfl-red-isolated.log','green':'numark-pfl-green.log','boundary':'idempotent per-deck PFL set'},
             {'red':'numark-pfl-disconnect-red.log','green':'numark-pfl-disconnect-green.log','boundary':'absolute level lifecycle cleanup; legacy toggle retained'}],
         'physical_new_image':'NOT RUN','ota':'NOT RUN','signed_packaging':'NOT RUN'}
assert len(summary['ui_demo_perform_frames'])==8
for old in ('pioneer_ddj_flx4','pioneer_ddj_400','ni_traktor_s2_mk3_full','hercules_djcontrol_inpulse_500'):
    path=f'controllers/{old}/profile.s3bin'
    assert subprocess.check_output(['git','show','d878c066:'+path])==(root/path).read_bytes()
    summary.setdefault('unchanged_existing_profile_sha256',{})[old]=hashlib.sha256((root/path).read_bytes()).hexdigest()
(cache/'integration-matrix.json').write_text(json.dumps(matrix,indent=2)+'\n',encoding='utf-8')
(cache/'integration-checks.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
print(json.dumps({'matrix':matrix,'checks':summary},indent=2))
