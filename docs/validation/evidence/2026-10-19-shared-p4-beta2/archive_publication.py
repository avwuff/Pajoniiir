import hashlib,json,re,shutil,subprocess
from pathlib import Path
ROOT=Path.cwd();HERE=Path(__file__).resolve().parent
OUT=ROOT/'docs/validation/evidence/2026-10-19-shared-p4-beta2';OUT.mkdir(exist_ok=True)
def read(name):return json.loads((HERE/name).read_text(encoding='utf-8-sig'))
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,obj):path.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
assert read('downloaded-public-audit.json')['complete'] and read('public-download-audit.json')['complete']
assert not (HERE/'package-source-2f2b0555').exists()
public=read('published-release.json');assert public['draft'] is False and public['prerelease'] is True and len(public['assets'])==16
for name in ('assembled-targets.json','final-assets.json','local-audit.json','downloaded-draft-audit.json','downloaded-public-audit.json',
 'public-download-audit.json','draft-release.json','published-release.json','publish-request.json',
 'stable-release-before.json','stable-release-after.json','beta1-before.json','beta1-after.json',
 'stable-ota-before.json','stable-ota-after.json','jc1060-package.log','assembly.log','assembly-v2.log','assembly-v3.log',
 'documentation-prepublication.log','documentation-final.log','assemble_assets.py','finalize_assets.py','audit_release.py','verify_publication.py','download_public.py'):
 shutil.copyfile(HERE/name,OUT/name)
for name in ('release-manifest.json','release-manifest.sig','SHA256SUMS.txt','SHA256SUMS.sig','RELEASE-NOTES.md'):
 dest='release-notes.txt' if name=='RELEASE-NOTES.md' else name
 shutil.copyfile(HERE/'assembly-v3/assets'/name,OUT/dest)
before=(HERE/'README-before.md').read_text(encoding='utf-8-sig');now=(ROOT/'README.md').read_text(encoding='utf-8')
def images(t):return sorted(re.findall(r'(?:src|srcset)="([^"]+)"|!\[[^\]]*\]\(([^)]+)\)',t))
assert images(before)==images(now) and len(images(now))==20
oldmanual=subprocess.check_output(['git','show','HEAD:docs/index.html'],text=True,encoding='utf-8')
assert images(oldmanual)==images((ROOT/'docs/index.html').read_text(encoding='utf-8'))
save(OUT/'publication-audit.json',{
 'complete':True,'tag':'shared-p4-beta.2','release_id':public['id'],'url':public['html_url'],
 'published_utc':public['published_at'],'draft':False,'prerelease':True,'make_latest':False,
 'distribution_source_sha':'d259a71ae716217a6448c374bf52c2c798d67ce8',
 'annotated_tag_object':'9ed56d5521fe7c818bb67fe5ec805755b226eff0',
 'per_board_sources':{r['board_id']:r['source_sha'] for r in read('assembled-targets.json')},
 'exact_m3_and_jc4880_binaries_and_signed_bundles_reused':True,'firmware_rebuilt_for_distribution':False,
 'assets':16,'zip_inventory_entries':222,'public_downloads_without_authentication':16,
 'all_hashes_signatures_metadata_debug_recovery_profile_checks':'PASS',
 'readme_original_image_references_preserved':20,'manual_source_image_references_preserved':True,
 'stable_latest_release_unchanged':'M2.5 /404235364, all asset IDs/sizes/digests match',
 'beta1_assets_unchanged':True,'public_ota_latest_json_sha256':sha(HERE/'stable-ota-after.json'),
 'public_ota_latest_json_bytes_unchanged':(HERE/'stable-ota-before.json').read_bytes()==(HERE/'stable-ota-after.json').read_bytes(),
 'hosted_ci':'NOT RUN; explicit operator quota waiver, skip-ci commit and no new run',
 'pages_deployment':'NOT RUN; user-manual source updated only, quota waiver retained',
 'temporary_packaging_checkout_removed':True,
 'physical_scope':'M3 exact image full-hour accepted with retained late investigation; JC4880 exact short smoke accepted and soak waived; JC1060 software-only, hardware deferred to Kayrosen; S2 operator-closed',
 'no_contributor_message_sent':True,
 'assembly_context':'First marker-only secret scan matched TLS delimiter strings, not a complete key; second incomplete attempt detected nested-inventory coverage. Final assembly-v3 independently verifies all222 entries. Incomplete assets never uploaded.'})
shutil.copyfile(__file__,OUT/'archive_publication.py')
(OUT/'.gitattributes').write_text('* -text whitespace=-blank-at-eol,-blank-at-eof,space-before-tab,cr-at-eol\n*.sig binary\n',encoding='utf-8')
save(OUT/'sha256.json',{p.name:sha(p) for p in sorted(OUT.iterdir()) if p.is_file() and p.name!='sha256.json'})
print('PASS publication evidence archived; README/manual references preserved; exact-source packaging checkout removed')
