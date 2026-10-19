"""Assemble retained, per-source beta2 images without rebuilding firmware."""
import hashlib,importlib.util,json,re,shutil,struct,subprocess,sys,zipfile
from pathlib import Path

ROOT=Path.cwd(); HERE=Path(__file__).resolve().parent; WORK=HERE/'assembly-v3'; ASSETS=WORK/'assets'
TAG='shared-p4-beta.2'; DIST='d259a71ae716217a6448c374bf52c2c798d67ce8'
assert subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()==DIST
assert not subprocess.check_output(['git','status','--porcelain'],text=True).strip()
assert not ASSETS.exists(),'Never overwrite assembled release assets'
ASSETS.mkdir(parents=True)
def git(*args):return subprocess.check_output(['git',*args],text=True).strip()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,obj):path.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
def copy(source,dest):dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,dest)
def inventory(folder):
 paths=sorted(p for p in folder.rglob('*') if p.is_file() and p!=folder/'SHA256SUMS.txt')
 (folder/'SHA256SUMS.txt').write_text(''.join(f'{sha(p)}  {p.relative_to(folder).as_posix()}\n' for p in paths),encoding='utf-8')
def zip_dir(folder,name):
 inventory(folder); dest=ASSETS/name
 with zipfile.ZipFile(dest,'x',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in sorted(folder.rglob('*')):
   if p.is_file():
    assert p.suffix.lower() not in ('.pem','.key') and 'private' not in p.name.lower(),p
    # TLS parsers embed PEM delimiter strings. Reject complete key payloads,
    # not a bare parser delimiter followed by NUL or unrelated binary text.
    assert not re.search(rb'-----BEGIN ((?:EC |RSA |OPENSSH |ENCRYPTED )?PRIVATE KEY)-----[\r\n]+[A-Za-z0-9+/=\r\n ]+-----END \1-----',p.read_bytes()),p
    z.write(p,p.relative_to(folder).as_posix())
 with zipfile.ZipFile(dest) as z:
  assert z.testzip() is None
  names=set(z.namelist());assert all(not n.startswith('/') and '..' not in Path(n).parts for n in names)
  entries=[]
  for line in z.read('SHA256SUMS.txt').decode().splitlines():
   digest,name=line.split('  ',1);assert hashlib.sha256(z.read(name)).hexdigest()==digest
   entries.append(name)
  assert names==set(entries)|{'SHA256SUMS.txt'}
 return {'file':dest.name,'size':dest.stat().st_size,'sha256':sha(dest)}
key=ROOT/'firmware/common/ota_manifest/keys/ddj_ota_release_public.der'
assert sha(key)=='125ef6eb776729e3018ecc564317254febec4a9a2bf34892e99d57f10bc02e43'
sign=ROOT/'tools/ota_signing.py'
def verify(*args):subprocess.run([sys.executable,str(sign),*map(str,args)],check=True)

spec=importlib.util.spec_from_file_location('compiler',ROOT/'tools/controller_profile/compile_profile.py')
compiler=importlib.util.module_from_spec(spec);spec.loader.exec_module(compiler)
profiles=WORK/'controller-profiles';profiles.mkdir(); profile_records=[]
for binary in sorted((ROOT/'controllers').glob('*/profile.s3bin')):
 source=binary.with_name('profile.json');compiled=compiler.compile_profile(json.loads(source.read_text(encoding='utf-8')))
 assert compiled==binary.read_bytes(),f'Stale compiled controller fixture: {binary}'
 folder=profiles/binary.parent.name;folder.mkdir()
 for filename in ('profile.s3bin','profile.json','README.md'):
  if (binary.parent/filename).exists():copy(binary.parent/filename,folder/filename)
 profile_records.append({'profile':binary.parent.name,'json_sha256':sha(source),'binary_sha256':sha(binary),'bytes':len(compiled)})
copy(ROOT/'LICENSE',profiles/'LICENSE')
save(profiles/'SOURCE.json',{'source_sha':DIST,'profiles':profile_records,'scope':'Compiled S3CP fixtures; physical qualification remains controller/image/topology-specific. S2 is operator-closed; ordinary M3 downloads do not use the experimental role swap.'})
zip_dir(profiles,'controller-profiles.zip')

boards=[
 ('jc4880','main-deck-p4','0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527',
  ROOT/'releases/jc4880-cache-bound-0a33ec1f-20261019/pajoniiir-M2.5-249-g0a33ec1f',
  ROOT/'.cache/idf-jc4880-headphones-20261018',
  'Exact supplied image installed VALID ota_0;60.234-s/93-sample mixed-rate MT/loop/CUE/control/audio/display smoke,1902 MIDI,all22 strict deltas0,output-late0; operator confirms all requested functions. Long soak explicitly waived / NOT RUN.'),
 ('jc1060','main-deck-jc1060','2f2b05558ccb68238d7228f59e52cd6c7d06e12f',
  HERE/'pajoniiir-jc1060-M2.5-246-g2f2b0555',ROOT/'.cache/idf-jc1060-integration-20261014',
  'Local IDF6.0.2 incremental build/board/software verification only. Supplied image NOT installed or physically accepted; startup/Ethernet/HTTP, SD warm-reset, USB MAIN reconnect and listening gates deferred to Kayrosen.'),
 ('m3','main-deck-m3','1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f',
  ROOT/'releases/m3-cache-bound-1556e8d7-20261018/pajoniiir-m3-M3-dev-g1556e8d7dfcb',
  ROOT/'.cache/idf-m3-status-observer-20261018',
  'Exact supplied image installed VALID ota_1; bounded loop/control/CUE checks and3600.286-s/5301-sample dual-MT mixed-rate hour with all22 strict deltas0 and full operator audio/display confirmation. Output-late24/max12988us/sample-observed3-per60s cluster remains investigation; not zero-late or unrestricted reliability PASS.')]
records=[]
for board,project,source,package,build,physical in boards:
 assert not git('diff','--name-only',source,DIST,'--','firmware','tests','tools','controllers')
 assert not subprocess.run(['git','merge-base','--is-ancestor',source,'origin/codex/shared-p4-core-m3']).returncode
 identity=json.loads((build/'board_build_identity.json').read_text())
 assert identity=={'source_sha':source,'source_dirty':0}
 manifest=json.loads((package/'manifest.json').read_text(encoding='utf-8-sig'));target,=manifest['targets']
 assert manifest['source_sha']==source and target['project']==project
 image=(package/target['file']).read_bytes()
 assert image==(build/target['file']).read_bytes()
 assert len(image)==target['size']<=0x380000 and hashlib.sha256(image).hexdigest()==target['sha256']
 assert image[0]==0xe9 and struct.unpack_from('<H',image,12)[0]==0x12 and struct.unpack_from('<I',image,32)[0]==0xabcd5432
 assert image[80:112].split(b'\0')[0].decode()==project
 assert image[48:80].split(b'\0')[0].decode()==manifest['release_version']
 assert source.encode() in image
 assert sha(package/'debug'/f'{project}.elf')==target['image_elf_sha256']==image[176:208].hex()
 assert sha(package/target['ota_bundle'])==target['bundle_sha256']
 verify('verify-bundle','--public-key',key,'--input',package/target['ota_bundle'])
 verify('verify-file','--public-key',key,'--input',package/'manifest.json','--signature',package/'manifest.sig')
 config=(build/'config/sdkconfig.h').read_text()
 for flag in ('BSP_M3_HS_CONTROLLER','CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC','USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM','AUDIO_RECORDER_ENABLED','AUDIO_RECORDER_EXPERIMENTAL_BUILD','PAJONIIIR_SD_IDLE_WAIT','PAJONIIIR_DJ_OVERVIEW','DDJ_OTA_STARTUP_TIMEOUT_TEST','DDJ_OTA_FORCE_ROLLBACK_TEST'):
  assert f'#define CONFIG_{flag} 1' not in config,(board,flag)
 desc=json.loads((build/'project_description.json').read_text())
 assert desc['project_name']==project and desc['project_version']==manifest['release_version']
 kit=WORK/f'{board}-beta-kit';kit.mkdir()
 for name in (target['file'],target['ota_bundle'],'manifest.json','manifest.sig'):copy(package/name,kit/name)
 copy(build/'board_build_identity.json',kit/'board_build_identity.json')
 copy(Path(desc['config_file']),kit/'sdkconfig')
 copy(build/'config/sdkconfig.h',kit/'sdkconfig.h')
 copy(ROOT/'firmware'/project/'dependencies.lock',kit/'dependencies.lock')
 copy(ROOT/'LICENSE',kit/'LICENSE');copy(key,kit/key.name)
 for name in ('flasher_args.json','ota_data_initial.bin','bootloader/bootloader.bin','partition_table/partition-table.bin'):
  copy(build/name,kit/'recovery'/name)
 # Preserve flash_files relative paths inside recovery; normal OTA uses only the signed bundle.
 copy(package/target['file'],kit/'recovery'/target['file'])
 flash=json.loads((kit/'recovery/flasher_args.json').read_text())
 assert all((kit/'recovery'/p).is_file() for p in flash['flash_files'].values())
 shutil.copytree(profiles,kit/'controller-profiles')
 notices=[]
 candidates=[ROOT/'LICENSE',Path(desc['idf_path'])/'LICENSE']
 for component in desc['build_component_paths']:
  folder=Path(component)
  if folder.is_dir():
   for p in folder.iterdir():
    if p.is_file() and (p.name.upper().startswith(('LICENSE','COPYING','NOTICE')) or p.name=='DJ_UI_NOTICE.md'):candidates.append(p)
 for relative in git('ls-files','firmware').splitlines():
  p=ROOT/relative
  if p.is_file() and (p.name.upper().startswith(('LICENSE','COPYING','NOTICE')) or p.name=='DJ_UI_NOTICE.md'):candidates.append(p)
 for i,p in enumerate(sorted(set(candidates))):
  dest=kit/'LICENSES'/f'{i:03d}-{p.parent.name}-{p.name}';copy(p,dest)
  notices.append({'file':dest.relative_to(kit).as_posix(),'original_component':p.parent.name,'sha256':sha(dest)})
 save(kit/'LICENSES/INDEX.json',notices)
 if board=='jc1060':copy(ROOT/'docs/JC1060_BETA_HANDOFF_20261011.md',kit/'JC1060-HANDOFF.md')
 record={'board_id':board,'project':project,'source_sha':source,'version':manifest['release_version'],
  'image_bytes':len(image),'image_sha256':target['sha256'],'bundle_bytes':target['bundle_size'],'bundle_sha256':target['bundle_sha256'],
  'image_elf_sha256':target['image_elf_sha256'],'ordinary_configuration':True,'physical_acceptance':physical,
  'dependency_lock_sha256':sha(kit/'dependencies.lock'),'sdkconfig_sha256':sha(kit/'sdkconfig'),
  'software_build_mode':'Retained ESP-IDF6.0.2 incremental build; full host Qualification on equivalent1556e8d7 production/test sources; no fresh release rebuild or hosted CI',
  'source_trees':{folder:git('rev-parse',f'{source}:{folder}') for folder in ('firmware','tests','tools','controllers')}}
 save(kit/'BETA-IDENTITY.json',record)
 (kit/'README.md').write_text(f'''# Pajoniiir {TAG}: {board}

Project: {project}; embedded version: {manifest['release_version']}.
Build source: {source}. Distribution source: {DIST}.
Acceptance: {physical}

Verify the signed release manifest and SHA256SUMS against the trusted public key
fingerprint125ef6eb776729e3018ecc564317254febec4a9a2bf34892e99d57f10bc02e43.
Kit inventories and signed per-board manifests identify every supplied image.
The included public key alone does not establish trust. Use only {target['ota_bundle']}
in this board's signed local OTA UI, with stopped decks and stable power.
Before playback set headphone LEVEL low and raise gradually; FLX4's corrected
cue signal is12.041 dB above the old transport-attenuated signal.

The recovery/ folder is for verified wired provisioning/recovery only. Its
flasher_args.json is relative to that folder. Check actual flash layout and
board identity; never erase the chip or overwrite partition settings blindly.
Raw BIN, bootloader, partitions and otadata are NOT signed web-OTA payloads.
Matching ELF/MAP/configuration are in {board}-debug.zip. Profiles are separate
uploads and require the matching controller and firmware topology.

Ordinary M3: FLX4 USB2 FS/root1, storage USB3 HS/root0, PCM5102A MAIN48000;
no Hercules/S2 HS-controller role swap. Historical M3 main-deck-p4 identity
requires the preserving wired migration before main-deck-m3 OTA. M3-dev has
no automatic pull discovery. JC4880: controller root1/storage root0,
PCM5102A MAIN selected by its board policy. JC1060: controller root0/storage
root1, controller MAIN/cue and Ethernet, no Wi-Fi or PCM5102A pin conflict.

Stable M2.5, prior beta assets and public OTA channels stay unchanged.
S2 remains operator-closed; JC4880 soak waived; JC1060 hardware NOT RUN.
Do not turn software/packaging evidence into wider physical qualification.
See https://github.com/dvucinozd/Pajoniiir/blob/{TAG}/docs/validation/SHARED_P4_BETA_2_20261019.md
and https://github.com/dvucinozd/Pajoniiir/blob/{TAG}/docs/OTA-UPDATE.md
''',encoding='utf-8')
 record['kit']=zip_dir(kit,f'{board}-beta-kit.zip')
 debug=WORK/f'{board}-debug';shutil.copytree(package/'debug',debug)
 record['debug']=zip_dir(debug,f'{board}-debug.zip')
 copy(package/target['ota_bundle'],ASSETS/target['ota_bundle'])
 records.append(record)
assert len({tuple(sorted(r['source_trees'].items())) for r in records})==1,'Cross-board production source drift'
copy(key,ASSETS/key.name)
save(HERE/'assembled-targets.json',records)
print(json.dumps(records,indent=2))
print('PASS unchanged binaries, signatures, source equivalence, embedded metadata, debug ELF, profile compilation and ZIP inventories')
