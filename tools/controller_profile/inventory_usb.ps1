[CmdletBinding()]
param(
    [ValidatePattern('^[0-9A-Fa-f]{4}$')][string]$VendorId = '17CC',
    [Parameter(Mandatory = $true)][string]$OutFile
)

$ErrorActionPreference = 'Stop'
$taskDestination = [IO.Path]::GetFullPath($OutFile)
if (Test-Path -LiteralPath $taskDestination) { throw "Evidence already exists: $taskDestination" }
$taskDevices = @(Get-PnpDevice -PresentOnly | Where-Object {
    $_.InstanceId -match "VID_$VendorId(?:&|\\)"
} | ForEach-Object {
    [ordered]@{
        name = $_.FriendlyName
        instance_id = $_.InstanceId
        class = $_.Class
        status = $_.Status
    }
})
$taskEvidence = [ordered]@{
    schema = 'controller-pnp-inventory-v1'
    utc = [DateTime]::UtcNow.ToString('o')
    requested_vendor_id = $VendorId.ToUpperInvariant()
    enumeration = $(if ($taskDevices.Count) { 'present' } else { 'not_found' })
    devices = $taskDevices
    raw_usb_descriptors_captured = $false
    note = 'PnP presence is not USB endpoint, MIDI input or UAC compatibility evidence.'
}
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($taskDestination)) | Out-Null
$taskJson = $taskEvidence | ConvertTo-Json -Depth 5
# CreateNew prevents a concurrent run from overwriting an earlier snapshot.
$taskStream = [IO.File]::Open($taskDestination, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write)
try {
    $taskBytes = [Text.UTF8Encoding]::new($false).GetBytes($taskJson + "`n")
    $taskStream.Write($taskBytes, 0, $taskBytes.Length)
} finally { $taskStream.Dispose() }
Write-Output $taskJson
