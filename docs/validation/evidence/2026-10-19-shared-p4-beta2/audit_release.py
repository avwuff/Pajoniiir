"""Independent read-only release/ZIP/signature audit; works on downloaded copies."""
import argparse,hashlib,importlib.util,json,struct,zipfile
from pathlib import Path
ROOT=Path.cwd();HERE=Path(__file__).resolve().parent
parser=argparse.ArgumentParser();parser.add_argument('--assets',type=Path,default=HERE/'assembly-v3/assets');parser.add_argument('--output',type=Path,default=HERE/'local-audit.json')
args=parser.parse_args();assets=args.assets
def digest(data):return hashlib.sha256(data).hexdigest()
spec=importlib.util.spec_from_file_location('signing',ROOT/'tools/ota_signing.py')
signing=importlib.util.module_from_spec(spec);spec.loader.exec_module(signing)
trusted=ROOT/'firmware/common/ota_manifest/keys/ddj_ota_release_public.der'
assert digest(trusted.read_bytes())=='125ef6eb776729e3018ecc564317254febec4a9a2bf34892e99d57f10bc02e43'
assert (assets/trusted.name).read_bytes()==trusted.read_bytes()
key=signing._load_public(trusted)
for name in ('release-manifest','SHA256SUMS'):
 suffix='.json' if name=='release-manifest' else '.txt'
 signing._raw_verify(key,(assets/(name+'.sig')).read_bytes(),(assets/(name+suffix)).read_bytes())
expected=json.loads((HERE/'final-assets.json').read_text())
assert {p.name for p in assets.iterdir() if p.is_file()}=={r['file'] for r in expected}
for r in expected:
 data=(assets/r['file']).read_bytes();assert len(data)==r['size'] and digest(data)==r['sha256'],r['file']
summed=[]
for line in (assets/'SHA256SUMS.txt').read_text().splitlines():
 sha,name=line.split('  ',1);assert digest((assets/name).read_bytes())==sha;summed.append(name)
assert set(summed)=={r['file'] for r in expected}-{'SHA256SUMS.txt','SHA256SUMS.sig'}
manifest=json.loads((assets/'release-manifest.json').read_text())
assert manifest['tag']=='shared-p4-beta.2' and manifest['distribution_source_sha']=='d259a71ae716217a6448c374bf52c2c798d67ce8'
for r in manifest['assets']:
 assert (assets/r['file']).stat().st_size==r['size'] and digest((assets/r['file']).read_bytes())==r['sha256']
checked_zip_entries=0
def verify_zip(z):
 global checked_zip_entries
 assert z.testzip() is None
 names=z.namelist();assert len(names)==len(set(names))
 assert all(not n.startswith('/') and '..' not in Path(n).parts and '\\' not in n for n in names)
 entries=[]
 for line in z.read('SHA256SUMS.txt').decode().splitlines():
  sha,name=line.split('  ',1);assert digest(z.read(name))==sha;entries.append(name)
 assert set(names)==set(entries)|{'SHA256SUMS.txt'}
 checked_zip_entries+=len(entries)
for p in assets.glob('*.zip'):
 with zipfile.ZipFile(p) as z:verify_zip(z)
for target in manifest['targets']:
 board=target['board_id'];project=target['project']
 with zipfile.ZipFile(assets/f'{board}-beta-kit.zip') as kit,zipfile.ZipFile(assets/f'{board}-debug.zip') as debug:
  image=kit.read(project+'.bin');bundle=kit.read(project+'.ddjota')
  assert len(image)==target['image_bytes'] and digest(image)==target['image_sha256']
  assert bundle==(assets/(project+'.ddjota')).read_bytes() and digest(bundle)==target['bundle_sha256']
  assert image[80:112].split(b'\0')[0].decode()==project and image[48:80].split(b'\0')[0].decode()==target['version']
  assert target['source_sha'].encode() in image and len(image)<=0x380000
  assert digest(debug.read(project+'.elf'))==image[176:208].hex()==target['image_elf_sha256']
  identity=json.loads(kit.read('board_build_identity.json'));assert identity=={'source_sha':target['source_sha'],'source_dirty':0}
  kitid=json.loads(kit.read('BETA-IDENTITY.json'));assert kitid['source_sha']==target['source_sha'] and kitid['physical_acceptance']==target['physical_acceptance']
  signed=json.loads(kit.read('manifest.json').decode('utf-8-sig'));assert signed['source_sha']==target['source_sha']
  signing._raw_verify(key,kit.read('manifest.sig'),kit.read('manifest.json'))
  parsed=signing.inspect_bundle(bundle,key);assert parsed['project']==project and parsed['version']==target['version']
  flash=json.loads(kit.read('recovery/flasher_args.json'))
  assert all('recovery/'+file in kit.namelist() for file in flash['flash_files'].values())
  assert kit.read('recovery/'+project+'.bin')==image
  assert digest(kit.read('sdkconfig'))==target['sdkconfig_sha256'] and digest(kit.read('dependencies.lock'))==target['dependency_lock_sha256']
assert len({tuple(sorted(t['source_trees'].items())) for t in manifest['targets']})==1
result={'complete':True,'assets':len(expected),'zip_entries_hashed':checked_zip_entries,
 'release_and_sums_signatures':'PASS','all_board_bundle_manifest_signatures':'PASS',
 'all_image_metadata_debug_identity_recovery_inventories':'PASS','source_equivalence':'PASS',
 'physical_scope':'Retained exact M3/JC4880 images; JC1060 software-only, not new or transferred physical acceptance',
 'asset_hashes':expected}
args.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print('PASS',len(expected),'assets;',checked_zip_entries,'ZIP inventory entries; signatures/image/project/source/debug/recovery/configuration checks')
