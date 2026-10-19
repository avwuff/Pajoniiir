$ErrorActionPreference='Stop'
Set-Location 'D:/AI/Pajoniiir-shared-p4-core'
$env:M3_CACHE_SOAK_OUTPUT=$PSScriptRoot
$taskExitCode=1
try {
 & 'C:/Espressif/python_env/idf6.0_py3.13_env/Scripts/python.exe' -u .cache/m3-cache-bound-20261018/final_soak.py 3600 1> "$PSScriptRoot/stdout.log" 2> "$PSScriptRoot/stderr.log"
 $taskExitCode=$LASTEXITCODE
} catch { $_ | Out-String | Add-Content "$PSScriptRoot/wrapper-error.log" }
finally { @{exit_code=$taskExitCode;ended_utc=[DateTime]::UtcNow.ToString('o');wrapper_pid=$PID} | ConvertTo-Json | Set-Content "$PSScriptRoot/wrapper-exit.json" }
exit $taskExitCode
