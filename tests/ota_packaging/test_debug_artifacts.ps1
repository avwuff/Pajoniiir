$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Import-Module (Join-Path $repo 'tools/OtaReleaseHelpers.psm1') -Force
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('pajoniiir-debug-archive-' + [guid]::NewGuid().ToString('N'))
$build = Join-Path $scratch 'build'
$output = Join-Path $scratch 'release'
$sha = 'a' * 40
function Reject([scriptblock]$Action, [string]$Pattern) {
    $rejected = $false
    try { & $Action | Out-Null } catch { $rejected = $_.Exception.Message -match $Pattern }
    if (-not $rejected) { throw "Expected debug archive rejection: $Pattern" }
}
try {
    New-Item -ItemType Directory -Path (Join-Path $build 'config') -Force | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $build 'main-deck-m3.elf'), [byte[]](1,2,3,4))
    'map-fixture' | Set-Content (Join-Path $build 'main-deck-m3.map')
    '{"project_name":"main-deck-m3"}' | Set-Content (Join-Path $build 'project_description.json')
    (@{source_sha=$sha;source_dirty=0} | ConvertTo-Json) | Set-Content (Join-Path $build 'board_build_identity.json')
    '#define CONFIG_TEST 1' | Set-Content (Join-Path $build 'config/sdkconfig.h')
    $elf = (Get-FileHash (Join-Path $build 'main-deck-m3.elf')).Hash.ToLowerInvariant()
    Reject { Save-OtaDebugArtifacts $build $output main-deck-m3 $sha ('b'*64) } 'ELF does not match'
    if (Test-Path $output) { throw 'Mismatched ELF created an archive' }
    $manifest = Save-OtaDebugArtifacts $build $output main-deck-m3 $sha $elf
    $m = Get-Content $manifest -Raw | ConvertFrom-Json
    if ($m.files.Count -ne 5 -or $m.source_sha -cne $sha -or $m.image_elf_sha256 -cne $elf) { throw 'Incomplete debug manifest' }
    foreach ($record in $m.files) {
        $path = Join-Path (Join-Path $output 'debug') $record.file
        if ((Get-FileHash $path).Hash.ToLowerInvariant() -cne $record.sha256) { throw 'Bad archive hash' }
    }
    $first = (Get-FileHash $manifest).Hash
    Save-OtaDebugArtifacts $build $output main-deck-m3 $sha $elf | Out-Null
    if ((Get-FileHash $manifest).Hash -cne $first) { throw 'Idempotent archive changed bytes' }
    'different-map' | Set-Content (Join-Path $build 'main-deck-m3.map')
    Reject { Save-OtaDebugArtifacts $build $output main-deck-m3 $sha $elf } 'Conflicting archived debug artifact'
    if ((Get-FileHash $manifest).Hash -cne $first) { throw 'Conflict changed manifest' }
    Remove-Item -LiteralPath (Join-Path $build 'main-deck-m3.map')
    Reject { Save-OtaDebugArtifacts $build $output main-deck-m3 $sha $elf } 'Missing debug artifact'
    Reject { Save-OtaDebugArtifacts $build $output main-deck-m3 'bad' $elf } 'Invalid debug artifact identity'
    Write-Host 'Debug artifact preservation tests passed.'
} finally {
    $resolved = [IO.Path]::GetFullPath($scratch)
    $temp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    if (-not $resolved.StartsWith($temp, [StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected test cleanup path' }
    if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
}
