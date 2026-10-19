[CmdletBinding()]
param([string]$PublicKey = '', [string]$Python = 'python')
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($PSScriptRoot)
if (-not $PublicKey) { $PublicKey = Join-Path $root 'verification/ddj_ota_release_public.der' }
$signer = Join-Path $root 'verification/ota_signing.py'
function Invoke-Verifier([string[]]$ToolArguments) {
    $output = & $Python $signer @ToolArguments
    if ($LASTEXITCODE -ne 0) { throw 'Signature or bundle verification failed' }
    return $output
}
Invoke-Verifier @('verify-file','--public-key',$PublicKey,'--input',(Join-Path $root 'SHA256SUMS.txt'),'--signature',(Join-Path $root 'SHA256SUMS.sig')) | Out-Null
$seen = @{}
foreach ($line in Get-Content -LiteralPath (Join-Path $root 'SHA256SUMS.txt')) {
    if ($line -notmatch '^([0-9a-f]{64})  (.+)$') { throw 'Invalid inventory line' }
    $expected, $relative = $Matches[1], $Matches[2]
    if ($relative -match '(^/|\\|:|(^|/)\.\.?(/|$))' -or $seen.ContainsKey($relative)) { throw 'Unsafe or duplicate inventory path' }
    $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
    if (-not $path.StartsWith($root + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Path outside kit' }
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $relative" }
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -cne $expected) { throw "Hash mismatch: $relative" }
    $seen[$relative] = $true
}
$actual = @(Get-ChildItem -LiteralPath $root -Recurse -Force -File | Where-Object { $_.FullName -ne (Join-Path $root 'SHA256SUMS.txt') -and $_.FullName -ne (Join-Path $root 'SHA256SUMS.sig') })
if ($actual.Count -ne $seen.Count) { throw 'Unlisted extra files are present' }
$identity = Get-Content -LiteralPath (Join-Path $root 'IDENTITY.json') -Raw | ConvertFrom-Json
$manifest = Get-Content -LiteralPath (Join-Path $root 'manifest.json') -Raw | ConvertFrom-Json
if ($identity.project -cne 'main-deck-jc1060' -or $identity.board_id -cne 'jc1060' -or $identity.source_dirty -ne $false -or $manifest.targets.Count -ne 1) { throw 'Wrong board/build identity' }
if ($manifest.source_sha -cne $identity.source_sha -or $manifest.release_version -cne $identity.version) { throw 'Source/version mismatch' }
Invoke-Verifier @('verify-file','--public-key',$PublicKey,'--input',(Join-Path $root 'manifest.json'),'--signature',(Join-Path $root 'manifest.sig')) | Out-Null
$target = $manifest.targets[0]
$bundleInfo = @(Invoke-Verifier @('verify-bundle','--public-key',$PublicKey,'--input',(Join-Path $root $target.ota_bundle)))
foreach ($item in @("project=$($identity.project)","version=$($identity.version)","sha256=$($identity.image_sha256)",'chip_id=18','key_id=rel-001','target=p4')) {
    if ($bundleInfo -cnotcontains $item) { throw "Signed bundle identity mismatch: $item" }
}
if ($target.sha256 -cne $identity.image_sha256 -or $target.bundle_sha256 -cne $identity.bundle_sha256 -or $target.image_elf_sha256 -cne $identity.image_elf_sha256) { throw 'Manifest hash identity mismatch' }
foreach ($entry in @(@($target.file,$identity.image_sha256),@($target.ota_bundle,$identity.bundle_sha256),@('debug/main-deck-jc1060.elf',$identity.image_elf_sha256))) {
    if ((Get-FileHash -LiteralPath (Join-Path $root $entry[0]) -Algorithm SHA256).Hash.ToLowerInvariant() -cne $entry[1]) { throw 'Image/bundle/ELF mismatch' }
}
$debugIdentity = Get-Content -LiteralPath (Join-Path $root 'debug/board_build_identity.json') -Raw | ConvertFrom-Json
if ($debugIdentity.source_sha -cne $identity.source_sha -or $debugIdentity.source_dirty -ne 0) { throw 'Debug source mismatch' }
Write-Output "PASS signed inventory, $($seen.Count) file hashes, manifest/bundle, JC1060 source/version and matching ELF"
Write-Output 'Offline package verification only; installation and physical acceptance NOT RUN.'
