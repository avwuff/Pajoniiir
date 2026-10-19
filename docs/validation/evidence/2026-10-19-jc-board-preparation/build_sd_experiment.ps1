$ErrorActionPreference='Stop'
. C:\Espressif\tools\Microsoft.v6.0.2.PowerShell_profile.ps1
$repoPath=(Get-Location).Path
$casePath=Join-Path $repoPath '.cache/jc-board-preparation/sd-experiment'
New-Item -ItemType Directory -Path $casePath -Force | Out-Null
$sourceSha=(& git rev-parse HEAD).Trim()
$lockPath=Join-Path $repoPath 'firmware/main-deck-jc1060/dependencies.lock'
$lockHash=(Get-FileHash $lockPath -Algorithm SHA256).Hash
& idf.py -C "$repoPath/firmware/main-deck-jc1060" -B "$casePath/build" -DIDF_TARGET=esp32p4 "-DSDKCONFIG=$casePath/sdkconfig" -DPAJONIIIR_SD_COLD_POWER_EXPERIMENT=ON build *> "$casePath/build.log"
$buildCode=$LASTEXITCODE
$verifyCode=$null
if($buildCode -eq 0){
 & python tools/check_board_build.py --project main-deck-jc1060 --build "$casePath/build" --experimental-sd-power *>> "$casePath/build.log"
 $verifyCode=$LASTEXITCODE
}
$result=[ordered]@{source_sha=$sourceSha;build_exit=$buildCode;verifier_exit=$verifyCode;lock_unchanged=$lockHash -eq (Get-FileHash $lockPath -Algorithm SHA256).Hash}
if($buildCode -eq 0){
 $desc=Get-Content "$casePath/build/project_description.json" -Raw|ConvertFrom-Json
 $config=Get-Content "$casePath/build/config/sdkconfig.h" -Raw
 $commands=Get-Content "$casePath/build/compile_commands.json" -Raw
 $result.sd_experiment_enabled=$config.Contains('#define CONFIG_PAJONIIIR_SD_COLD_POWER_EXPERIMENT 1')
 $result.production_helper_compiled=$commands.Contains('sd_cold_power.c')
 $result.ppa_component_path=$desc.build_component_paths[[array]::IndexOf($desc.build_components,'esp_driver_ppa')]
 $result.image_bytes=(Get-Item "$casePath/build/main-deck-jc1060.bin").Length
 $result.image_sha256=(Get-FileHash "$casePath/build/main-deck-jc1060.bin" -Algorithm SHA256).Hash
 $result.board_build_identity=Get-Content "$casePath/build/board_build_identity.json" -Raw|ConvertFrom-Json
}
$result|ConvertTo-Json -Depth 5|Set-Content "$casePath/result.json"
$result|ConvertTo-Json -Depth 5
if($buildCode -ne 0 -or $verifyCode -ne 0 -or -not $result.lock_unchanged){Get-Content "$casePath/build.log" -Tail 35;exit 1}
