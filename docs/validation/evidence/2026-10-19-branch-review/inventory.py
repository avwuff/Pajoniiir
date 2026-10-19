import json,subprocess
from pathlib import Path
OUT=Path(__file__).resolve().parent
def git(*args,check=True):
 p=subprocess.run(['git',*args],capture_output=True,text=True,encoding='utf-8',errors='replace')
 if check and p.returncode:raise RuntimeError(p.stderr)
 return p
baseline=git('rev-parse','origin/master').stdout.strip();shared=git('rev-parse','HEAD').stdout.strip()
refs=['origin/codex/shared-p4-core-m3','origin/jc8012p4a1-v3','origin/kay/jc1060-hw-bench','origin/kay/new-ui','origin/numark-dj2go2-touch','codex/p4-apta-library']
records=[]
for ref in refs:
 slug=ref.replace('/','_'); tip=git('rev-parse',ref).stdout.strip();base=git('merge-base',baseline,tip).stdout.strip()
 changes=git('diff','--name-status',base,tip).stdout
 (OUT/(slug+'-changes.txt')).write_text(changes,encoding='utf-8')
 log=git('log','--format=%H %s',base+'..'+tip).stdout;(OUT/(slug+'-commits.txt')).write_text(log,encoding='utf-8')
 counts={};merges={};cherries={}
 for label,target in [('master',baseline),('shared',shared)]:
  counts[label]=[int(n) for n in git('rev-list','--left-right','--count',target+'...'+tip).stdout.split()]
  merge=git('merge-tree','--write-tree','--name-only','--no-messages',target,tip,check=False)
  assert merge.returncode in (0,1),(ref,merge.stderr)
  (OUT/(slug+'-merge-'+label+'.txt')).write_text(merge.stdout,encoding='utf-8')
  lines=merge.stdout.strip().splitlines();merges[label]={'exit':merge.returncode,'tree':lines[0] if lines else None,'conflict_paths':[line for line in lines[1:] if line.strip()]}
  cherry=git('cherry',target,tip).stdout.splitlines()
  cherries[label]={'patch_equivalent':sum(s.startswith('-') for s in cherry),'unique_patch':sum(s.startswith('+') for s in cherry)}
 record={'ref':ref,'source_sha':tip,'merge_base_master':base,'ahead_behind':counts,'cherry':cherries,'virtual_merge':merges,
  'unique_commits_since_master_base':len(log.splitlines()),'changed_files_since_master_base':len(changes.splitlines())}
 records.append(record)
 print(json.dumps({k:v for k,v in record.items() if k!='virtual_merge'},ensure_ascii=False))
(OUT/'inventory.json').write_text(json.dumps({'master':baseline,'shared':shared,'branches':records},indent=2)+'\n',encoding='utf-8')
