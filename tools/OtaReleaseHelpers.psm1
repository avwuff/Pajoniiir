function ConvertTo-EspAppVersion {
    param(
        [Parameter(Mandatory = $true)][string]$Version
    )

    if ([string]::IsNullOrEmpty($Version)) {
        throw "ESP application version must not be empty"
    }

    # esp_app_desc_t.version is a 32-byte NUL-terminated field. Trim by UTF-8
    # bytes, never by UTF-16 code units, so the signed manifest contains the
    # exact valid text that can fit in the image descriptor.
    $utf8 = [System.Text.UTF8Encoding]::new($false, $true)
    $candidate = $Version
    while ($candidate.Length -gt 0) {
        try {
            if ($utf8.GetByteCount($candidate) -le 31) {
                return $candidate
            }
        } catch [System.Text.EncoderFallbackException] {
            # Removing one UTF-16 code unit may temporarily leave a high
            # surrogate. Continue trimming until a valid boundary is reached.
        }
        $candidate = $candidate.Substring(0, $candidate.Length - 1)
    }

    throw "ESP application version has no valid UTF-8 prefix within 31 bytes"
}

function Resolve-OtaSigningPython {
    $candidates = @()
    if ($env:PAJONIIIR_OTA_PYTHON) {
        $candidates += $env:PAJONIIIR_OTA_PYTHON
    }
    if ($env:IDF_PYTHON_ENV_PATH) {
        $relativeExe = if ($env:OS -eq "Windows_NT") {
            "Scripts\python.exe"
        } else {
            "bin/python"
        }
        $candidates += Join-Path $env:IDF_PYTHON_ENV_PATH $relativeExe
    }
    if ($env:OS -eq "Windows_NT") {
        $candidates += "C:\Espressif\tools\python\v6.0.2\venv\Scripts\python.exe"
        $candidates += "C:\Espressif\python_env\idf6.0_py3.13_env\Scripts\python.exe"
        $candidates += "C:\Espressif\python_env\idf6.0_py3.11_env\Scripts\python.exe"
    }
    foreach ($name in @("python", "python3")) {
        foreach ($command in @(Get-Command $name -All -ErrorAction SilentlyContinue)) {
            if ($command.Source) { $candidates += $command.Source }
        }
    }

    foreach ($candidate in @($candidates | Select-Object -Unique)) {
        if (-not $candidate -or -not (Test-Path -LiteralPath $candidate)) { continue }
        $previousErrorAction = $ErrorActionPreference
        $ErrorActionPreference = "Continue"
        try {
            & $candidate -c "import cryptography" 2>&1 | Out-Null
            if ($LASTEXITCODE -eq 0) { return $candidate }
        } catch {
            # Try the next interpreter.
        } finally {
            $ErrorActionPreference = $previousErrorAction
        }
    }
    throw "No Python interpreter with a working cryptography module was found"
}

function Save-OtaDebugArtifacts {
    param(
        [Parameter(Mandatory = $true)][string]$BuildDir,
        [Parameter(Mandatory = $true)][string]$OutputDir,
        [Parameter(Mandatory = $true)][string]$Project,
        [Parameter(Mandatory = $true)][string]$SourceSha,
        [Parameter(Mandatory = $true)][string]$ExpectedElfSha256
    )
    if ($SourceSha -cnotmatch '^[0-9a-f]{40}$' -or
        $ExpectedElfSha256 -cnotmatch '^[0-9a-f]{64}$' -or
        $Project -notin @('main-deck-p4','main-deck-jc1060','main-deck-m3')) {
        throw 'Invalid debug artifact identity'
    }
    $names = @("$Project.elf", "$Project.map", 'project_description.json',
               'board_build_identity.json', 'config/sdkconfig.h')
    $records = foreach ($name in $names) {
        $path = Join-Path $BuildDir $name
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing debug artifact: $name" }
        [ordered]@{ file = [IO.Path]::GetFileName($name)
                    size = (Get-Item -LiteralPath $path).Length
                    sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() }
    }
    if ($records[0].sha256 -cne $ExpectedElfSha256) {
        throw 'Debug ELF does not match the application descriptor SHA256'
    }
    $identity = Get-Content -LiteralPath (Join-Path $BuildDir 'board_build_identity.json') -Raw | ConvertFrom-Json
    $description = Get-Content -LiteralPath (Join-Path $BuildDir 'project_description.json') -Raw | ConvertFrom-Json
    if ($identity.source_sha -cne $SourceSha -or $identity.source_dirty -ne 0 -or
        $description.project_name -cne $Project) { throw 'Debug build identity does not match packaging source/project' }
    $destination = Join-Path $OutputDir 'debug'
    # Do not replace an archived debug file with different bytes on a rerun.
    foreach ($record in $records) {
        $path = Join-Path $destination $record.file
        if ((Test-Path -LiteralPath $path) -and
            (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -cne $record.sha256) {
            throw "Conflicting archived debug artifact: $($record.file)"
        }
    }
    New-Item -ItemType Directory -Path $destination -Force | Out-Null
    for ($i = 0; $i -lt $names.Count; $i++) {
        $path = Join-Path $destination $records[$i].file
        if (-not (Test-Path -LiteralPath $path)) {
            Copy-Item -LiteralPath (Join-Path $BuildDir $names[$i]) -Destination $path
        }
        if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -cne $records[$i].sha256) {
            throw 'Debug artifact copy verification failed'
        }
    }
    $manifest = [ordered]@{ schema_version = 1; scope = 'local-debug-artifacts'
        project = $Project; source_sha = $SourceSha; image_elf_sha256 = $ExpectedElfSha256
        files = @($records) }
    $manifestPath = Join-Path $destination 'debug-manifest.json'
    $json = $manifest | ConvertTo-Json -Depth 5
    if (Test-Path -LiteralPath $manifestPath) {
        if ((Get-Content -LiteralPath $manifestPath -Raw).Trim() -cne $json.Trim()) {
            throw 'Conflicting archived debug manifest'
        }
    } else { $json | Set-Content -LiteralPath $manifestPath -Encoding utf8 }
    return $manifestPath
}

Export-ModuleMember -Function ConvertTo-EspAppVersion, Resolve-OtaSigningPython, Save-OtaDebugArtifacts
