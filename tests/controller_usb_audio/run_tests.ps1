param(
    [ValidateSet('JC4880', 'M3-HS')]
    [string[]]$Board = @('JC4880', 'M3-HS')
)
$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$Component = Join-Path $RepoRoot "firmware/p4-core/components/controller_usb_audio"
$BuildDir = Join-Path $PSScriptRoot "build"
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
foreach ($Policy in $Board) {
$Target = Join-Path $BuildDir "test_controller_usb_audio_stream_$Policy"
if ($env:OS -eq "Windows_NT") { $Target += ".exe" }
[string[]]$PolicyFlags = if ($Policy -eq 'JC4880') {
    @('-DCONFIG_PAJONIIIR_BOARD_JC4880=1')
} else {
    @('-DCONFIG_PAJONIIIR_BOARD_M3=1', '-DCONFIG_BSP_M3_HS_CONTROLLER=1',
      '-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC=1')
}
Write-Output "Production stream policy: $Policy"
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror `
    @PolicyFlags `
    "-I$PSScriptRoot/stubs" "-I$Component" "-I$Component/include" `
    "-I$RepoRoot/firmware/p4-core/components/service_log/include" `
    (Join-Path $PSScriptRoot "test_controller_usb_audio_stream.c") `
    (Join-Path $Component "controller_audio_ring.c") `
    (Join-Path $Component "controller_audio_resampler.c") `
    (Join-Path $Component "flx4_uac_packetizer.c") `
    (Join-Path $Component "flx4_uac_descriptors.c") `
    (Join-Path $Component "controller_uac2.c") -o $Target
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $Target
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
exit $LASTEXITCODE
