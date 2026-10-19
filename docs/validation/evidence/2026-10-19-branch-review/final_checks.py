import concurrent.futures, hashlib, json, os, subprocess
from pathlib import Path

root=Path.cwd(); out=root/'.cache/branch-review-20261019'
inventory=json.loads((out/'inventory.json').read_text())
def git(*args): return subprocess.check_output(['git',*args],text=True,encoding='utf-8')
def checks(branch):
    sha=branch['source_sha']
    p=subprocess.run(['gh','api',f'repos/dvucinozd/Pajoniiir/commits/{sha}/check-runs','--paginate'],capture_output=True,text=True,encoding='utf-8')
    if p.returncode: return {'ref':branch['ref'],'sha':sha,'error':p.stderr.strip()}
    data=json.loads(p.stdout)
    return {'ref':branch['ref'],'sha':sha,'total_count':data['total_count'],
            'checks':[{'name':c['name'],'status':c['status'],'conclusion':c['conclusion'],'url':c['html_url']} for c in data['check_runs']]}
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    statuses=list(pool.map(checks,inventory['branches']))
(out/'github-check-runs.json').write_text(json.dumps(statuses,indent=2)+'\n')

# Compile the test text bundled inside the Numark patch, without applying that patch.
# Its fixture-relative fopen requires a small, isolated overlay directory.
overlay=out/'numark-overlay'
cwd=overlay/'tests/controller_profile';cwd.mkdir(parents=True,exist_ok=True)
fixture=overlay/'controllers/numark_dj2go2_touch/profile.s3bin';fixture.parent.mkdir(parents=True,exist_ok=True)
fixture.write_bytes((out/'numark-source/profile.s3bin').read_bytes())
components=root/'firmware/p4-core/components'
args=['C:/msys64/ucrt64/bin/gcc.exe','-std=c11','-Wall','-Wextra','-Werror',
      '-DCONTROLLER_PROFILE_MANAGER_PC_TEST','-I'+str(root/'tests/controller_runtime/stubs')]
for name in ['controller_profile','controller_runtime','controller_led_runtime','controller_usb_host','control_link','controller_profile_manager']:
    args+=['-I'+str(components/name/'include')]
exe=cwd/'numark-review.exe'
args += [str(out/'numark-source/test-from-embedded-patch.c'),str(components/'controller_profile/controller_profile.c'),
         str(components/'controller_profile_manager/controller_profile_manager.c'),'-o',str(exe)]
env=os.environ.copy();env['PATH']+=';C:/msys64/ucrt64/bin'
p=subprocess.run(args,capture_output=True,text=True,env=env,cwd=cwd)
result={'compile_exit':p.returncode,'compile_output':p.stdout+p.stderr,
        'scope':'Test extracted from embedded patch only, against current production parser/manager; not a branch-provided runner or PFL deck actor test'}
if p.returncode==0:
    r=subprocess.run([str(exe)],cwd=cwd,capture_output=True,text=True,env=env)
    result.update(run_exit=r.returncode,run_output=r.stdout+r.stderr)
(out/'numark-extracted-test.json').write_text(json.dumps(result,indent=2)+'\n')
sd=git('show','origin/jc8012p4a1-v3:firmware/boards/jc-common/board_peripherals.c')
(out/'jc8012-sd-review.txt').write_text(sd,encoding='utf-8')
for line in statuses: print(line['ref'],[(c['name'],c['conclusion']) for c in line.get('checks',[])],line.get('error',''))
print('Numark extracted test',result)
