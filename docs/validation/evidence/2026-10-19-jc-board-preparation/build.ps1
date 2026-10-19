param([string[]]$Projects = @('main-deck-m3'), [switch]$ForceConfigure)
$ErrorActionPreference = 'Stop'
$repoPath = (Get-Location).Path
$evidencePath = Join-Path $repoPath '.cache/jc-board-preparation'
. C:\Espressif\tools\Microsoft.v6.0.2.PowerShell_profile.ps1
$idfVersion = (& idf.py --version | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $idfVersion -notmatch 'ESP-IDF v6\.0\.2') { throw "Wrong IDF: $idfVersion" }
$sourceSha = (& git rev-parse HEAD).Trim()
foreach ($projectName in $Projects) {
    $projectPath = Join-Path $repoPath "firmware/$projectName"
    $lockPath = Join-Path $projectPath 'dependencies.lock'
    $lockHash = (Get-FileHash -LiteralPath $lockPath -Algorithm SHA256).Hash
    $logPath = Join-Path $evidencePath "integration-build-$projectName.log"
    if ($ForceConfigure) {
        & idf.py -C $projectPath reconfigure *> $logPath
        if ($LASTEXITCODE -ne 0) { throw "Reconfigure failed: $projectName" }
        & idf.py -C $projectPath build *>> $logPath
    } else {
        & idf.py -C $projectPath build *> $logPath
    }
    $buildCode = $LASTEXITCODE
    $verifyCode = $null
    $uiSelected = $null
    $ppaPath = $null
    if ($buildCode -eq 0) {
        & python tools/check_board_build.py --project $projectName --build "$projectPath/build" *>> $logPath
        $verifyCode = $LASTEXITCODE
        $description = Get-Content "$projectPath/build/project_description.json" -Raw | ConvertFrom-Json
        $uiSelected = 'ui_next' -in $description.build_components
        $ppaIndex = [array]::IndexOf($description.build_components, 'esp_driver_ppa')
        if ($ppaIndex -ge 0) { $ppaPath = $description.build_component_paths[$ppaIndex] }
    }
    $result = [ordered]@{
        source_sha = $sourceSha; idf_version = $idfVersion; project = $projectName
        build_exit = $buildCode; verifier_exit = $verifyCode
        lock_sha256 = $lockHash; lock_unchanged = $lockHash -eq (Get-FileHash -LiteralPath $lockPath -Algorithm SHA256).Hash
        ui_next_selected = $uiSelected; ppa_component_path = $ppaPath
    }
    if ($buildCode -eq 0) {
        $imagePath = "$projectPath/build/$projectName.bin"
        $result.image_bytes = (Get-Item -LiteralPath $imagePath).Length
        $result.image_sha256 = (Get-FileHash -LiteralPath $imagePath -Algorithm SHA256).Hash
        $result.board_build_identity = Get-Content "$projectPath/build/board_build_identity.json" -Raw | ConvertFrom-Json
        $result.project_version = $description.project_version
    }
    $result | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $evidencePath "integration-build-$projectName.json")
    $result | ConvertTo-Json -Depth 6
    if ($buildCode -ne 0 -or $verifyCode -ne 0 -or -not $result.lock_unchanged -or $uiSelected) {
        Get-Content -LiteralPath $logPath -Tail 35
        throw "Integration build gate failed: $projectName"
    }
}

